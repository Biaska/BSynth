#include "synth.h"
#include <math.h>
#include <stdlib.h>

// Helper function to convert MIDI note to frequency
static float midiToHz(int midiNote)
{
    return 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
}

// Function to initialize a specific oscillator type
static void synth_setOscillatorType(Synth *synth, OscillatorType oscType)
{
    // Assign function pointers based on the oscillator type
    switch (oscType)
    {
    case OSC_SINE:
        synth->tick = (OscillatorTickFn)tCycle_tick;
        synth->init = (OscillatorInitFn)tCycle_init;
        synth->free = (OscillatorFreeFn)tCycle_free;
        synth->setFreq = (OscillatorSetFreqFn)tCycle_setFreq;
        synth->setPhase = (OscillatorSetSampleRateFn)tCycle_setSampleRate;
        synth->setSample = (OscillatorSetPhaseFn)tCycle_setPhase;
        break;
    case OSC_SQUARE:
        synth->tick = (OscillatorTickFn)tSquare_tick;
        synth->init = (OscillatorInitFn)tSquare_init;
        synth->free = (OscillatorFreeFn)tSquare_free;
        synth->setFreq = (OscillatorSetFreqFn)tSquare_setFreq;
        synth->setPhase = (OscillatorSetSampleRateFn)tSquare_setSampleRate;
        synth->setSample = (OscillatorSetPhaseFn)tSquare_setPhase;
        break;
    case OSC_SAW:
        synth->tick = (OscillatorTickFn)tSawtooth_tick;
        synth->init = (OscillatorInitFn)tSawtooth_init;
        synth->free = (OscillatorFreeFn)tSawtooth_free;
        synth->setFreq = (OscillatorSetFreqFn)tSawtooth_setFreq;
        synth->setPhase = (OscillatorSetSampleRateFn)tSawtooth_setSampleRate;
        synth->setSample = (OscillatorSetPhaseFn)tSawtooth_setPhase;
        break;

    default:
        return;
    }

    void *type;

    // Initialize all voices with the selected oscillator type
    for (int i = 0; i < MAX_VOICES; i++)
    {
        if (oscType == OSC_SINE)
        {
            tCycle type;
        } else if (oscType == OSC_SQUARE)
        {
            tSquare type;
        } else if (oscType == OSC_SAW) {
            tSawtooth type;
        }
        synth->oscillators[i] = type;
        synth->init(synth->oscillators[i], synth->leaf);
    }
}

// Initialize the synthesizer
void synth_init(Synth *synth, LEAF *leaf, int sampleRate, OscillatorType oscType)
{
    synth->leaf = leaf;
    tSimplePoly_init(&synth->poly, MAX_VOICES, leaf);
    synth_setOscillatorType(synth, oscType);
}

// Free synth from mempool memory
void synth_free(Synth *synth)
{
    // TODO
}

// Handle MIDI Note On
void synth_noteOn(Synth *synth, int note, uint8_t velocity)
{
    int voice = tSimplePoly_noteOn(&synth->poly, note, velocity);
    if (voice >= 0 && synth->setFreq)
    {
        float freq = midiToHz(note);
        synth->setFreq(synth->oscillators[voice], freq);
    }
}

// Handle MIDI Note Off
void synth_noteOff(Synth *synth, int note)
{
    int voice = tSimplePoly_noteOff(&synth->poly, note);
    synth->setFreq(synth->oscillators[voice], 0);
}