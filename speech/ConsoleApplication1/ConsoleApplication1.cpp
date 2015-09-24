// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "wavfile.h"
#include <math.h>

#define S_RATE  (44100)
#define BUF_SIZE (S_RATE*2) /* 2 second buffer */
int buffer[BUF_SIZE];

int _tmain(int argc, _TCHAR* argv[])
{
	
	int i;
    float t;
    float amplitude = 32000;
    float freq_Hz = 440;
    float phase=0;
 
    float freq_radians_per_sample = freq_Hz*2*3.14159265358979323846/S_RATE;
 
    /* fill buffer with a sine wave */
    for (i=0; i<BUF_SIZE; i++)
    {
        phase += freq_radians_per_sample;
        buffer[i] = (int)(amplitude * sin(phase));
    }
 
    write_wav("test.wav", BUF_SIZE, buffer, S_RATE);
	return 0;
}

