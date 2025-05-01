#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <usbcfg.h>
#include <camera/po8030.h>

//RGB LED used for interaction with plotImage Python code
#include <leds.h>

#include "camera.h"
#include "main.h"
#include "telemetry.h"

static float distance_cm = 0;
static uint16_t line_position = IMAGE_BUFFER_SIZE/2;    //middle


//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */
uint16_t extract_line_width(uint8_t *buffer){

    uint16_t i = 0, begin = 0, end = 0, width = 0;
    uint8_t stop = 0, wrong_line = 0, line_not_found = 0;
    uint32_t mean = 0;

    static uint16_t last_width = PXTOCM/GOAL_DISTANCE;

    //performs an average
    for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
        mean += buffer[i];
    }
    mean /= IMAGE_BUFFER_SIZE;

    do{
        wrong_line = 0;
        //search for a begin
        while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
        { 
            //the slope must at least be WIDTH_SLOPE wide and is compared
            //to the mean of the image
            if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
            {
                begin = i;
                stop = 1;
            }
            i++;
        }
        //if a begin was found, search for an end
        if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
        {
            stop = 0;
            
            while(stop == 0 && i < IMAGE_BUFFER_SIZE)
            {
                if(buffer[i] > mean && buffer[i-WIDTH_SLOPE] < mean)
                {
                    end = i;
                    stop = 1;
                }
                i++;
            }
            //if an end was not found
            if (i > IMAGE_BUFFER_SIZE || !end)
            {
                line_not_found = 1;
            }
        }
        else//if no begin was found
        {
            line_not_found = 1;
        }

        //if a line too small has been detected, continues the search
        if(!line_not_found && (end-begin) < MIN_LINE_WIDTH){
            i = end;
            begin = 0;
            end = 0;
            stop = 0;
            wrong_line = 1;
        }
    }while(wrong_line);

    if(line_not_found){
        begin = 0;
        end = 0;
        width = last_width;
    }else{
        last_width = width = (end - begin);
        line_position = (begin + end)/2; //gives the line position.
    }

    //sets a maximum width or returns the measured width
    if((PXTOCM/width) > MAX_DISTANCE){
        return PXTOCM/MAX_DISTANCE;
    }else{
        return width;
    }
}

bool detect_color(uint8_t *buffer){
    uint32_t mean = 0;
    uint16_t i_min = 0;
    uint16_t i_max = 0;

    for(volatile uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
        mean += buffer[i];
        if (buffer[i] < buffer[i_min]){
            i_min = i;
        }
        if (buffer[i] > buffer[i_max]){
            i_max = i;
        }   
    }

    mean /= IMAGE_BUFFER_SIZE;

    volatile uint8_t drop = buffer[i_max] - buffer[i_min];

    if (drop > mean/4){
        return 0; 
    }
    else {
        return 1;
    }
}


color_detection_t extract_color(uint8_t *red_buffer, uint8_t *green_buffer, uint8_t *blue_buffer){
    bool red = detect_color(red_buffer);
    bool green = detect_color(green_buffer);
    bool blue = detect_color(blue_buffer);

    // bool red = 0;
    // bool green = 0;
    // bool blue = 0;
    if (green){
        return GREEN_COLOR;
    }
    else if (red){
        return RED_COLOR;
    }
    else if (blue){
        return BLUE_COLOR;
    }
    else {
        return BLACK_COLOR;
    }
}

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //Takes pixels 0 to IMAGE_BUFFER_SIZE of the lines USED_LINE and USED_LINE + 1 (minimum 2 lines because reasons)
    po8030_advanced_config(FORMAT_RGB565, 0, USED_LINE, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
    dcmi_enable_double_buffering();
    dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
    dcmi_prepare();

    while(1){
        //starts a capture
        dcmi_capture_start();
        //waits for the capture to be done
        wait_image_ready();
        //signals an image has been captured
        chBSemSignal(&image_ready_sem);
    }
}

static THD_WORKING_AREA(waProcessImage, 2048);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    uint8_t *img_buff_ptr;
    uint8_t red_buffer[IMAGE_BUFFER_SIZE] = {0};
    uint8_t green_buffer[IMAGE_BUFFER_SIZE] = {0};
    uint8_t blue_buffer[IMAGE_BUFFER_SIZE] = {0};

    uint16_t lineWidth = 0;

    bool send_to_computer = true;

    while(1){
        //waits until an image has been captured
        chBSemWait(&image_ready_sem);
        //gets the pointer to the array filled with the last image in RGB565    
        img_buff_ptr = dcmi_get_last_image_ptr();

        
        for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
            //extracts 5 MSbits of the MSbyte (First byte in big-endian format)
            //takes nothing from the second byte
            red_buffer[i/2] = (uint8_t)img_buff_ptr[i] & 0xF8;

            //extracts 3 LSbits of the first byte and the 3 MSbits of second byte
            green_buffer[i/2] = (((uint8_t)img_buff_ptr[i] & 0x07) << 5 )
                               + (((uint8_t)img_buff_ptr[i+1] & 0xE0) >> 3);

            //extracts 5 LSbits of the LSByte (Second byte in big-endian format)
            //and rescale to 8 bits
            //takes nothing from the first byte
            blue_buffer[i/2] = ((uint8_t)img_buff_ptr[i+1] & 0x1F) << 3;
        }

        //search for a line in the image and gets its width in pixels

        color_detection_t colorDetected = extract_color(red_buffer, green_buffer, blue_buffer);

        switch (colorDetected) {
		    case RED_COLOR:
                //Analyze a buffer with a drop in the pixel intensity
                //lineWidth = extract_line_width(green_buffer);
                epuck_printf("Red\n");
				break;
            
            case GREEN_COLOR:
                //lineWidth = extract_line_width(red_buffer);
                epuck_printf("Green\n");
				break;

            case BLUE_COLOR:
                //lineWidth = extract_line_width(red_buffer);
                epuck_printf("Blue\n");
				break;

            case BLACK_COLOR:
                //lineWidth = extract_line_width(red_buffer);
                epuck_printf("Black\n");
				break;
        //
        }
        //lineWidth = extract_line_width(red_buffer);

        if(lineWidth){
            distance_cm = PXTOCM/lineWidth;
        }

        //if(send_to_computer){
            //sends to the computer the image
            //SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
        //}
        //invert the bool
        send_to_computer = !send_to_computer;
    }
}


float get_distance_cm(void){
    return distance_cm;
}

uint16_t get_line_position(void){
    return line_position;
}

void process_image_start(void){
    chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
    chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}

 
void camera_init(void)
{
    //starts the camera
    dcmi_start();
    po8030_start();
    process_image_start();
}