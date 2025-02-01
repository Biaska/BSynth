#include "leaf.h"
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MEM_SIZE 500000
#define AUDIO_BUFFER_SIZE 128
#define MAX_VOICES 8 // Max polyphony

float randomNumber()
{
    return (float)rand() / (float)RAND_MAX;
}

// LEAF & audio objects
LEAF leaf;
tSimplePoly poly;
tCycle oscillators[MAX_VOICES];
char myMemory[MEM_SIZE];

// Simulated MIDI input (C Major chord)
int midiNotes[] = {42, 46, 48}; // MIDI Notes: C4, E4, G4
uint8_t velocity = 100;         // Fixed velocity

float midiToHz(int midiNote)
{
    return 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
}

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
        float sample = 0.0f;
        int activeVoices = 0;

        for (int j = 0; j < MAX_VOICES; j++)
        {
            if (tSimplePoly_isOn(poly, j))
            {
                sample += tCycle_tick(oscillators[j]);
                activeVoices++;
            }
        }

        // Prevent clipping
        if (activeVoices > 0)
        {
            sample /= activeVoices;
        }

        *out++ = sample;
    }

    return paContinue;
}

int main()
{

/*============================   Iintialize Leaf   =============================*/

    // Initialize LEAF
    printf("Initializing LEAF...\n");
    LEAF_init(&leaf, 48000, myMemory, MEM_SIZE, &randomNumber);

/*============================   Handle Oscillator   =============================*/

    // Initialize polyphonic MIDI handler
    tSimplePoly_init(&poly, MAX_VOICES, &leaf);

    // Initialize oscillators
    for (int i = 0; i < MAX_VOICES; i++)
    {
        tCycle_init(&oscillators[i], &leaf);
    }

    // Simulate MIDI note-on messages
    for (int i = 0; i < 3; i++) // Playing 3 notes (C, E, G)
    {
        int voice = tSimplePoly_noteOn(poly, midiNotes[i], velocity);
        if (voice >= 0) // Ensure the voice is assigned
        {
            float freq = midiToHz(midiNotes[i]);
            tCycle_setFreq(oscillators[voice], freq);
        }
    }

/*============================   Handle Audio Stream   =============================*/

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

/*============================   Stop Program   =============================*/
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    printf("Audio stream stopped.\n");

    return 0;
}
