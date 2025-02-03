#ifndef SYNTH_H
#define SYNTH_H

#include "leaf.h"

#define MAX_VOICES 8

// Function pointer for oscillator tick function
typedef float (*OscillatorTickFn)(void *osc);
typedef void  (*OscillatorFreeFn)(void *osc);
typedef void  (*OscillatorInitFn)(void *osc, LEAF *leaf);
typedef void  (*OscillatorSetFreqFn)(void *osc, float freq);
typedef void  (*OscillatorSetSampleFn)(void *osc, float sr);
typedef void  (*OscillatorSetPhaseFn)(void *osc, float phase);

// Struct for voice handling
typedef struct
{
    LEAF *leaf;                         // LEAF instance
    tSimplePoly poly;                   // Poly Handler
    void *oscillators[MAX_VOICES];      // Generic oscillator pointers
    OscillatorTickFn tick;              // Tick function for specific osc
    void *oscType;                      // Osc type
    OscillatorFreeFn free;              // Osc free function
    OscillatorInitFn init;              // Osc init function
    OscillatorSetSampleFn setSample;    // Osc set sample function
    OscillatorSetFreqFn setFreq;        // Osc set freq function
    OscillatorSetPhaseFn setPhase;      // Osc set phase function
} Synth;

// Fix these functions

void synth_init(Synth *synth, LEAF *leaf, int sampleRate, OscillatorType oscType);
void synth_setOscillatorType(Synth *synth, OscillatorType oscType);
void synth_noteOn(Synth *synth, int note, uint8_t velocity);
void synth_noteOff(Synth *synth, int note);

#endif // SYNTH_H
