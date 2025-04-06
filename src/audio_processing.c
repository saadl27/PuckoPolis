#include <ch.h>
#include <hal.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <arm_math.h>
#include <string.h>

#include "audio_processing.h"
#include "main.h"
#include "fft.h"
#include "communications.h"


//semaphore
static BSEMAPHORE_DECL(sendToComputer_sem, TRUE);

//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];
static float micFront_cmplx_input[2 * FFT_SIZE];
static float micBack_cmplx_input[2 * FFT_SIZE];
//Arrays containing the computed magnitude of the complex numbers
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];
static float micFront_output[FFT_SIZE];
static float micBack_output[FFT_SIZE];

/*
*	Callback called when the demodulation of the four microphones is done.
*	We get 160 samples per mic every 10ms (16kHz)
*	
*	params :
*	int16_t *data			Buffer containing 4 times 160 samples. the samples are sorted by micro
*							so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
*	uint16_t num_samples	Tells how many data we get in total (should always be 640)
*/
void processAudioData(int16_t *data, uint16_t num_samples){

	/*
	*
	*	We get 160 samples per mic every 10ms
	*	So we fill the samples buffers to reach
	*	1024 samples, then we compute the FFTs.
	*
	*/
	static int k = 0;

	for (uint16_t i=0 ; i < num_samples/4; ++i){
		micRight_cmplx_input[2*i+160*k] = (float) data[4*i];
		micLeft_cmplx_input[2*i+160*k] = (float) data[4*i+1];
		micBack_cmplx_input[2*i+160*k] = (float) data[4*i+2];
		micFront_cmplx_input[2*i+160*k] = (float) data[4*i+3];

		micRight_cmplx_input[2*i+160*k+1] = 0;
		micLeft_cmplx_input[2*i+160*k+1] = 0;
		micBack_cmplx_input[2*i+160*k+1] = 0;
		micFront_cmplx_input[2*i+160*k+1] = 0;
		k++;
		if (i+160*k == 2*FFT_SIZE){
			//arm_cfft_f32(&arm_cfft_sR_f32_len1024, micRight_cmplx_input, 0, 1);
			//arm_cfft_f32(&arm_cfft_sR_f32_len1024, micFront_cmplx_input, 0, 1);
			//arm_cfft_f32(&arm_cfft_sR_f32_len1024, micBack_cmplx_input, 0, 1);
			//arm_cfft_f32(&arm_cfft_sR_f32_len1024, micFront_cmplx_input, 0, 1);
			doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
			doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
			doFFT_optimized(FFT_SIZE, micBack_cmplx_input);
			doFFT_optimized(FFT_SIZE, micFront_cmplx_input);

			arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
			arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);
			arm_cmplx_mag_f32(micBack_cmplx_input, micBack_output, FFT_SIZE);
			arm_cmplx_mag_f32(micFront_cmplx_input, micFront_output, FFT_SIZE);

			chBSemSignal(&sendToComputer_sem);
			k=0;
			break;
		}
	}
}

void wait_send_to_computer(void){
	chBSemWait(&sendToComputer_sem);
}

float* get_audio_buffer_ptr(BUFFER_NAME_t name){
	if(name == LEFT_CMPLX_INPUT){
		return micLeft_cmplx_input;
	}
	else if (name == RIGHT_CMPLX_INPUT){
		return micRight_cmplx_input;
	}
	else if (name == FRONT_CMPLX_INPUT){
		return micFront_cmplx_input;
	}
	else if (name == BACK_CMPLX_INPUT){
		return micBack_cmplx_input;
	}
	else if (name == LEFT_OUTPUT){
		return micLeft_output;
	}
	else if (name == RIGHT_OUTPUT){
		return micRight_output;
	}
	else if (name == FRONT_OUTPUT){
		return micFront_output;
	}
	else if (name == BACK_OUTPUT){
		return micBack_output;
	}
	else{
		return NULL;
	}
}
