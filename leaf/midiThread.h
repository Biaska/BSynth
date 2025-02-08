#ifndef MIDITHREAD_H
#define MIDITHREAD_H

#include <alsa/asoundlib.h>
#include "sineSynth.h"

// Structure to pass data to the MIDI thread.
typedef struct {
    snd_seq_t *seq_handle;
    SineSynth *synth;
    int local_port;
} MidiThreadData;

int get_midi_client();
snd_seq_t *midi_init();
void *midi_thread_func(void *arg);

#endif