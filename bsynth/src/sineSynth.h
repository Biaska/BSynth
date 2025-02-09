#ifndef SINESYNTH_H
#define SINESYNTH_H

#include "leaf/leaf.h"

#define MAX_VOICES 8

// Function pointer for oscillator tick function
typedef float (*OscillatorTickFn)(tCycle osc);
typedef void  (*OscillatorFreeFn)(tCycle *osc);
typedef void  (*OscillatorInitFn)(tCycle *osc, LEAF *leaf);
typedef void  (*OscillatorSetFreqFn)(tCycle osc, float freq);
typedef void  (*OscillatorSetSampleRateFn)(tCycle osc, float sr);
typedef void  (*OscillatorSetPhaseFn)(tCycle osc, float phase);

// Struct for oscillator voice handling
typedef struct
{
    LEAF *leaf;                             // LEAF instance
    tSimplePoly poly;                       // Poly Handler
    tCycle *oscillators;         // Generic oscillator pointers
    OscillatorTickFn tick;                  // Tick function for specific osc
    OscillatorFreeFn free;                  // Osc free function
    OscillatorInitFn init;                  // Osc init function
    OscillatorSetSampleRateFn setSample;    // Osc set sample function
    OscillatorSetFreqFn setFreq;            // Osc set freq function
    OscillatorSetPhaseFn setPhase;          // Osc set phase function
} SineSynth;


SineSynth *synth_init(LEAF *leaf, int sampleRate);
void synth_free(SineSynth *synth);
int synth_getNumActiveVoices(SineSynth *synth);
float synth_tick(SineSynth *synth);
void synth_setOscillators(SineSynth *synth);
void synth_noteOn(SineSynth *synth, int note, uint8_t velocity);
void synth_noteOff(SineSynth *synth, int note);

#endif // SINESYNTH_H
