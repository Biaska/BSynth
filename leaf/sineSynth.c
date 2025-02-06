#include "sineSynth.h"
#include <math.h>
#include <stdlib.h>

tCycle oscillators[MAX_VOICES];

// Helper function to convert MIDI note to frequency
static float midiToHz(int midiNote)
{
    return 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
}

// Function to initialize a specific oscillator type
void synth_setOscillators(SineSynth *synth)
{   
    // Assign function pointers
    synth->tick = (OscillatorTickFn)tCycle_tick;
    synth->init = (OscillatorInitFn)tCycle_init;
    synth->free = (OscillatorFreeFn)tCycle_free;
    synth->setFreq = (OscillatorSetFreqFn)tCycle_setFreq;
    synth->setSample = (OscillatorSetSampleRateFn)tCycle_setSampleRate;
    synth->setPhase = (OscillatorSetPhaseFn)tCycle_setPhase;
    synth->oscillators = oscillators;

    // Initialize all voices with the selected oscillator type
    for (int i = 0; i < MAX_VOICES; i++)
    {
        synth->init(&synth->oscillators[i], synth->leaf);
    }
}


// Initialize the synthesizer
SineSynth *synth_init(LEAF *leaf, int sampleRate)
{
    SineSynth *synth = (SineSynth *)malloc(sizeof(SineSynth));
    synth->leaf = leaf;
    tSimplePoly_init(&synth->poly, MAX_VOICES, leaf);
    synth_setOscillators(synth);
    return synth;
}

// Free synth from mempool memory
void synth_free(SineSynth *synth)
{
    for (int i = 0; i<MAX_VOICES; i++)
    {
        if (synth->oscillators[i])
        {
            synth->free(&synth->oscillators[i]);
        }
    }

        tSimplePoly_free(&synth->poly);
    free(synth);
}

// Get the number of active voices
int synth_getNumActiveVoices(SineSynth *synth)
{
    return tSimplePoly_getNumActiveVoices(synth->poly);
}

// Handle tick for all oscillators
float synth_tick(SineSynth *synth)
{
    float sample = 0.0f;
    for (int i = 0; i < MAX_VOICES; i++)
    {
        if (tSimplePoly_isOn(synth->poly, i))
        {
            sample += synth->tick(synth->oscillators[i]);
        }
    }
    return sample;
}

// Handle MIDI Note On
void synth_noteOn(SineSynth *synth, int note, uint8_t velocity)
{
    int voice = tSimplePoly_noteOn(synth->poly, note, velocity);
    if (voice >= 0 && synth->setFreq)
    {
        float freq = midiToHz(note);
        synth->setFreq(synth->oscillators[voice], freq);
    }
}

// Handle MIDI Note Off
void synth_noteOff(SineSynth *synth, int note)
{
    int voice = tSimplePoly_noteOff(synth->poly, note);
    synth->setFreq(synth->oscillators[voice], 0);
}
