#include "leaf/leaf.h"
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "midiThread.h"
#include "sineSynth.h"

// gcc -g -Ileaf/Inc/ -Ileaf/src/ -Ileaf/Externals -IPortAudio -o sineSynthesizer sineSynthesizer.c sineSynth.c midiThread.c -L./leaf/build -lportaudio -lleaf -lm -lpthread -lasound

#define MEM_SIZE 500000
#define AUDIO_BUFFER_SIZE 128
#define SAMPLE_RATE 48000
#define MAX_VOICES 8 // Max polyphony

float randomNumber()
{
    return (float)rand() / (float)RAND_MAX;
}


// Simulated MIDI input (C Major chord)
int midiNotes[] = {60, 64, 67}; // MIDI Notes: C4, E4, G4
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
    SineSynth *synth = (SineSynth *)userData;

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
        // if (*out != 0.0f)
        // {
        //     printf("out: %f sample %f\n", *out, sample);
        // }
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
    SineSynth *synth = synth_init(&leaf, SAMPLE_RATE);
    printf("Synth initialized.\n");

/*============================   Initialize Midi Thread   =============================*/

    snd_seq_t *seq_handle = midi_init();
 

    // Set up and start the MIDI thread.
    MidiThreadData midiData;
    midiData.seq_handle = seq_handle;
    midiData.synth = synth;

    pthread_t midi_thread;
    if (pthread_create(&midi_thread, NULL, midi_thread_func, &midiData) != 0) {
        fprintf(stderr, "Error creating MIDI thread.\n");
        return 1;
    }
    printf("MIDI thread started.\n");

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


/*============================   Stop Program   =============================*/
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    printf("Audio stream stopped.\n");

    printf("Cleaning up synth...\n");
    synth_free(synth);

    return 0;
}