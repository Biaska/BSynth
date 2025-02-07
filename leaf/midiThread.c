#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "sineSynth.h"  // Your synth header with synth_noteOn / synth_noteOff prototypes
#include "midiThread.h"

int get_midi_client()
{
    // placeholder
    return 16;
}

snd_seq_t *midi_init()
{
    snd_seq_t *seq_handle;
    int err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
    if (err < 0) {
        fprintf(stderr, "Error opening sequencer: %s\n", snd_strerror(err));
        exit(1);
    }
    snd_seq_set_client_name(seq_handle, "BSynth");

    int in_port = snd_seq_create_simple_port(seq_handle, "Input Port",
                                             SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                             SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    if (in_port < 0) {
        fprintf(stderr, "Error creating sequencer port: %s\n", snd_strerror(in_port));
        snd_seq_close(seq_handle);
        exit(1);
    }
    printf("MIDI sequencer port created (client %d, port %d).\n", snd_seq_client_id(seq_handle), in_port);

    int MIDI_CLIENT = get_midi_client();
    int MIDI_PORT = 0;


    snd_seq_addr_t sender, dest;
    sender.client = MIDI_CLIENT;
    sender.port = MIDI_PORT;
    dest.client = snd_seq_client_id(seq_handle);
    dest.port = in_port;

    snd_seq_port_subscribe_t *subs;
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    snd_seq_port_subscribe_set_queue(subs, 1);
    snd_seq_port_subscribe_set_time_update(subs, 1);
    snd_seq_port_subscribe_set_time_real(subs, 1);
    err = snd_seq_subscribe_port(seq_handle, subs);

    if (err < 0) {
        fprintf(stderr, "Could not subscribe to %d:%d - %s\n",
                MIDI_CLIENT, MIDI_PORT, snd_strerror(err));
    }
    else {
        printf("Subscribed to %d:%d successfully.\n", MIDI_CLIENT, MIDI_PORT);
    }
    return seq_handle;
}

// MIDI event loop thread
void *midi_thread_func(void *arg)
{
    MidiThreadData *data = (MidiThreadData *)arg;
    snd_seq_t *seq_handle = data->seq_handle;
    SineSynth *synth = data->synth;
    snd_seq_event_t *event;

    while (1) {
        // This call blocks until an event is available.
        if (snd_seq_event_input(seq_handle, &event) >= 0) {
            if (event) {
                switch (event->type) {
                    case SND_SEQ_EVENT_NOTEON: {
                        int note = event->data.note.note;
                        int velocity = event->data.note.velocity;
                        // Some devices send Note On with velocity 0 to indicate Note Off.
                        if (velocity == 0) {
                            printf("MIDI: Note Off: note=%d\n", note);
                            synth_noteOff(synth, note);
                        } else {
                            printf("MIDI: Note On: note=%d vel=%d\n", note, velocity);
                            synth_noteOn(synth, note, (uint8_t)velocity);
                        }
                        break;
                    }
                    case SND_SEQ_EVENT_NOTEOFF: {
                        int note = event->data.note.note;
                        printf("MIDI: Note Off: note=%d\n", note);
                        synth_noteOff(synth, note);
                        break;
                    }
                    // You can add additional event types (e.g. CC) as needed.
                    default:
                        // For other events, just print a message.
                        printf("MIDI: Received event type %d\n", event->type);
                        break;
                }
                snd_seq_free_event(event);
            }
        }
    }

    return NULL;
}
