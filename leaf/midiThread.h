#include <asoundlib.h>
#include "sineSynth.h"

// Structure to pass data to the MIDI thread.
typedef struct {
    snd_seq_t *seq_handle;
    SineSynth *synth;
} MidiThreadData;