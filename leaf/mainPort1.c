#include "leaf.h"
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "synth.h"

#define MEM_SIZE 500000
#define AUDIO_BUFFER_SIZE 128
#define SAMPLE_RATE 48000
#define MAX_VOICES 8 // Max polyphony

float randomNumber()
{
    return (float)rand() / (float)RAND_MAX;
}


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
    Synth *synth = (Synth *)userData;

    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        int activeVoices = synth_getNumActiveVoices(synth);
        float sample = synth_tick(synth);

        // Prevent clipping
        if (activeVoices > 0)
        {
            sample /= activeVoices;
        }

        *out++ = sample;
        printf("out: %f sample %f\n", *out, sample);
    }

    return paContinue;
}

int main()
{

/*============================   Iintialize Leaf   =============================*/
    LEAF leaf;
    char mem[MEM_SIZE];

    // Initialize LEAF
    printf("Initializing LEAF...\n");
    LEAF_init(&leaf, SAMPLE_RATE, mem, MEM_SIZE, &randomNumber);
    printf("Leaf initialized.\n");

/*============================   Initialize Synth   =============================*/

    printf("Initializing synth...\n");
    Synth *synth = synth_init(&leaf, SAMPLE_RATE, OSC_SINE);
    printf("Synth initialized.\n");

    // simulate midi input
    for (int i = 0; i < 3; i++) // Playing 3 notes (C, E, G)
    {
        synth_noteOn(synth, midiNotes[i], velocity);
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
                               synth);            // No user data
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

    // simulate midi input stopping
    for (int i = 0; i < 3; i++) // Stopping 3 notes (C, E, G)
    {
        synth_noteOff(synth, midiNotes[i]);
    }


/*============================   Stop Program   =============================*/
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    printf("Audio stream stopped.\n");

    printf("Cleaning up synth...\n");
    synth_free(synth);

    return 0;
}