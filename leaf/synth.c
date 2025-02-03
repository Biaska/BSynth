#include "synth.h"
#include <math.h>
#include <stdlib.h>

// Helper function to get frequency from MIDI note
float midiToHz(int midiNote)
{
    return 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
}

// Initialize the synthesizer
void synth_init(Synth *synth, LEAF *leaf, int sampleRate, OscillatorType oscType)
{
    synth->leaf = leaf;
    synth->sampleRate = sampleRate;
    synth->oscType = oscType;
    tSimplePoly_init(&synth->poly, MAX_VOICES, leaf);

    // Initialize oscillators based on the selected type
    synth_setOscillatorType(synth, oscType);
}

// Change oscillator type
void synth_setOscillatorType(Synth *synth, OscillatorType oscType)
{
    synth->oscType = oscType;

    for (int i = 0; i < MAX_VOICES; i++)
    {
        if (synth->oscillators[i])
        {
            free(synth->oscillators[i]); // Free old oscillator
        }

        if (oscType == OSC_SINE)
        {
            tCycle *osc = (tCycle *)malloc(sizeof(tCycle));
            tCycle_init(osc, synth->leaf);
            synth->oscillators[i] = osc;
            synth->tickFunc[i] = (OscillatorTickFunc)tCycle_tick;
        }
        // Add square, saw oscillators here when implemented
    }
}

// Note On: Assigns note to a voice
void synth_noteOn(Synth *synth, int note, uint8_t velocity)
{
    int voice = tSimplePoly_noteOn(&synth->poly, note, velocity);
    if (voice >= 0)
    {
        float freq = midiToHz(note);
        tCycle_setFreq((tCycle *)synth->oscillators[voice], freq);
    }
}

// Note Off: Removes note from the polyphony handler
void synth_noteOff(Synth *synth, int note)
{
    tSimplePoly_noteOff(&synth->poly, note);
}
