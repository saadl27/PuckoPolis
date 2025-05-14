#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <usbcfg.h>
#include <camera/po8030.h>
#include <camera/dcmi_camera.h>

#include "modules/include/camera.h"
#include "modules/include/telemetry.h"
#include "modules/include/brain.h"
#include "main.h"


static uint16_t line_position = IMAGE_BUFFER_SIZE/2;    //middle


//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */
static void extract_line_width(uint8_t *buffer){

    uint16_t i = 0, begin = 0, end = 0;
    uint8_t stop = 0, wrong_line = 0, line_not_found = 0;
    uint32_t mean = 0;

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
    }else{
        line_position = (begin + end)/2; //gives the line position.
    }

    /* //sets a maximum width or returns the measured width
    if((PXTOCM/width) > MAX_DISTANCE){
        return PXTOCM/MAX_DISTANCE;
    }else{
        return width;
    } */
}

bool detect_color(uint8_t *buffer, color_detection_t color) {
    uint32_t mean = 0;
    uint16_t i_min = 0;
    uint16_t i_max = 0;

    for(uint16_t i = INDEX_OFFSET; i < IMAGE_BUFFER_SIZE - INDEX_OFFSET ; i++){
        mean += buffer[i];
        if (buffer[i] < buffer[i_min]){
            i_min = i;
        }
        if (buffer[i] > buffer[i_max]){
            i_max = i;
        }   
    }

    mean /= IMAGE_BUFFER_SIZE;

    uint32_t drop = (uint32_t) (buffer[i_max] - buffer[i_min]);
    float scaler = 0.0f;
    switch (color) {
        case RED_COLOR: scaler = RED_THRESHOLD_SCALE; break;
        case GREEN_COLOR: scaler = GREEN_THRESHOLD_SCALE; break;
        case BLUE_COLOR: scaler = BLUE_THRESHOLD_SCALE; break;
        default: break;
    }
    uint32_t threshold = scaler * mean;

    // if (color == BLUE_COLOR) epuck_printf("drop = %lu, threshold = %lu\n", drop, threshold);

    if (drop > threshold){
        return false; 
    } else {
        return true;
    }
}

color_detection_t extract_color(uint8_t *red_buffer, uint8_t *green_buffer, uint8_t *blue_buffer){
    bool red = detect_color(red_buffer, RED_COLOR);
    bool green = detect_color(green_buffer, GREEN_COLOR);
    bool blue = detect_color(blue_buffer, BLUE_COLOR);

    if (green && red && blue) {
        return WHITE_COLOR;
    }
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

static THD_WORKING_AREA(waCaptureImage, CAPTURE_STACK_SIZE);
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

static THD_WORKING_AREA(waProcessImage, PROCESS_STACK_SIZE);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t color_topic;
    color_msg_t color_values;

    MUTEX_DECL(color_topic_lock);
    CONDVAR_DECL(color_topic_condvar);
    messagebus_topic_init(&color_topic, &color_topic_lock, &color_topic_condvar, &color_values, sizeof(color_values));
    messagebus_advertise_topic(&bus, &color_topic, "/color");


    uint8_t *img_buff_ptr;
    uint8_t red_buffer[IMAGE_BUFFER_SIZE] = {0};
    uint8_t green_buffer[IMAGE_BUFFER_SIZE] = {0};
    uint8_t blue_buffer[IMAGE_BUFFER_SIZE] = {0};

    uint8_t counter = 0; // for color polling (filters out random noise)
    uint8_t red_count = 0, green_count = 0, blue_count = 0, black_count = 0, white_count = 0;
    color_detection_t last_colors[POLLING_COUNT];

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

        last_colors[counter++] = extract_color(red_buffer, green_buffer, blue_buffer);

        switch (last_colors[counter - 1]) {
            case RED_COLOR:
                //Analyze a buffer with a drop in the pixel intensity
                extract_line_width(green_buffer);
                break;
            
            case GREEN_COLOR:
                extract_line_width(red_buffer);
                break;

            case BLUE_COLOR:
                extract_line_width(red_buffer);
                break;

            case BLACK_COLOR:
                extract_line_width(red_buffer);
                break;
            default: break;
        }

   /*      if(lineWidth){
            distance_cm = PXTOCM/lineWidth;
        } */

        if (counter == POLLING_COUNT) {
            counter = 0;
            for (size_t i = 0; i < POLLING_COUNT; ++i) {
                switch (last_colors[i]) {
                    case RED_COLOR: red_count++; break;
                    case GREEN_COLOR: green_count++; break;
                    case BLUE_COLOR: blue_count++; break;
                    case BLACK_COLOR: black_count++; break;
                    case WHITE_COLOR: white_count++; break;
                }
            }
            
            char* clr = NULL;

            if (red_count > green_count && red_count > blue_count && 
                red_count > black_count && red_count > white_count) {
                color_values.color = RED_COLOR;
                clr = "red";
            } else if (green_count > red_count && green_count > blue_count &&
                 green_count > black_count && green_count > white_count) {
                color_values.color = GREEN_COLOR;
                clr = "green";
            } else if (blue_count > red_count && blue_count > green_count &&
                 blue_count > black_count && blue_count > white_count) {
                color_values.color = BLUE_COLOR;
                clr = "blue";
            } else if  (black_count > red_count && black_count > green_count &&
                 black_count > blue_count && black_count > white_count){
                color_values.color = BLACK_COLOR;
                clr = "black";
            } else {
                color_values.color = WHITE_COLOR;
                clr = "white";
            }

            red_count = 0;
            green_count = 0;
            blue_count = 0;
            black_count = 0;
            white_count = 0;

            epuck_printf("%s\n", clr);

            messagebus_topic_publish(&color_topic, &color_values, sizeof(color_values));
        }
    }
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