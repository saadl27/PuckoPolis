#ifndef FFT_H
#define FFT_H

#define FFT_SIZE 	1024

typedef struct complex_float{
	float real;
	float imag;
}complex_float;

void doFFT_optimized(uint16_t size, float* complex_buffer);

void doFFT_c(uint16_t size, complex_float* complex_buffer);

#endif /* FFT_H */