#include "leaf.h"
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 500000
#define AUDIO_BUFFER_SIZE 128

float randomNumber()
{
    return (float)rand() / (float)RAND_MAX;
}

LEAF leaf;
tCycle mySine;
char myMemory[MEM_SIZE];

// Audio callback function for PortAudio
static int audioCallback(const void *inputBuffer,
                         void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData)
{
    float *out = (float *)outputBuffer;

    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        *out++ = tCycle_tick(mySine); // Generate sine wave samples
    }

    return paContinue;
}

int main()
{
    // Initialize LEAF
    printf("Initializing LEAF...\n");
    LEAF_init(&leaf, 48000, myMemory, MEM_SIZE, &randomNumber);

    // Initialize sine wave oscillator
    tCycle_init(&mySine, &leaf);
    tCycle_setFreq(mySine, 80.0);

    // Initialize PortAudio
    printf("Initializing PortAudio...\n");
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    // Open audio stream
    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream,
                               0,                 // No input channels
                               1,                 // 1 output channel
                               paFloat32,         // 32-bit floating-point samples
                               48000,             // Sample rate
                               AUDIO_BUFFER_SIZE, // Frames per buffer
                               audioCallback,     // Callback function
                               NULL);             // No user data
    if (err != paNoError)
    {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    // Start the audio stream
    printf("Starting audio stream...\n");
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    printf("Playing sound. Press Enter to stop.\n");
    getchar(); // Wait for user input

    // Stop the audio stream
    Pa_StopStream(stream);
    Pa_CloseStream(stream);

    // Terminate PortAudio
    Pa_Terminate();
    printf("Audio stream stopped.\n");

    return 0;
}
