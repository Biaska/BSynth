#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "sineSynth.h"  // Your synth header with synth_noteOn / synth_noteOff prototypes
#include "midiThread.h"


/*
 * subscribe_to_port()
 *
 * Given the ALSA sequencer handle, the local (destination) port number,
 * and a remote client/port pair, this function subscribes to that port
 * if it is not our own port.
 */
void subscribe_to_port(snd_seq_t *seq_handle, int local_port, int client, int port)
{
    /* Do not subscribe to our own port */
    if (client == snd_seq_client_id(seq_handle))
        return;
    
    snd_seq_addr_t sender, dest;
    sender.client = client;
    sender.port  = port;
    dest.client  = snd_seq_client_id(seq_handle);
    dest.port    = local_port;
    
    snd_seq_port_subscribe_t *subs;
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    snd_seq_port_subscribe_set_queue(subs, 1);
    snd_seq_port_subscribe_set_time_update(subs, 1);
    snd_seq_port_subscribe_set_time_real(subs, 1);
    
    int err = snd_seq_subscribe_port(seq_handle, subs);
    if (err < 0) {
        fprintf(stderr, "Error subscribing to %d:%d - %s\n", client, port, snd_strerror(err));
    }
    else {
        printf("Subscribed to %d:%d successfully.\n", client, port);
    }
}


/*
 * scan_and_subscribe_ports()
 *
 * This function iterates over all current ALSA clients and their ports.
 * For each port that has the capabilities to send MIDI (i.e. it supports
 * READ and subscription for READ), it calls subscribe_to_port() to
 * subscribe to it.
 */
void scan_and_subscribe_ports(snd_seq_t *seq_handle, int local_port)
{
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    /* Iterate over all clients */
    snd_seq_client_info_set_client(cinfo, -1);
    while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);
        /* Iterate over each port for the current client */
        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
            unsigned int cap = snd_seq_port_info_get_capability(pinfo);
            /* Check if the port can send events (READ) and accepts subscriptions for READ */
            if ((cap & SND_SEQ_PORT_CAP_READ) && (cap & SND_SEQ_PORT_CAP_SUBS_READ)) {
                int port = snd_seq_port_info_get_port(pinfo);
                subscribe_to_port(seq_handle, local_port, client, port);
            }
        }
    }
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
    printf("MIDI sequencer port created (client %d, port %d).\n",
     snd_seq_client_id(seq_handle), in_port);

    /* Scan and subscribe to all currently available MIDI ports */
    scan_and_subscribe_ports(seq_handle, in_port);

    return seq_handle;
}

/*
 * midi_thread_func()
 *
 * This is the event loop thread. In addition to handling Note On/Off events,
 * we also listen for SND_SEQ_EVENT_PORT_START events, which are generated when
 * a new port appears (i.e. when a MIDI device is plugged in). When such an event
 * is received, we check the new port’s capabilities and subscribe to it if appropriate.
 *
 * Make sure that your MidiThreadData structure (in midiThread.h) includes an int
 * for the local port number (e.g. `int local_port;`).
 */
void *midi_thread_func(void *arg)
{
    MidiThreadData *data = (MidiThreadData *)arg;
    snd_seq_t *seq_handle = data->seq_handle;
    SineSynth *synth = data->synth;
    int local_port = data->local_port;  // Our port number, set in midi_init()
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
                    case SND_SEQ_EVENT_CLIENT_START: 
                    case SND_SEQ_EVENT_PORT_START: {
                        int client = event->data.addr.client;
                        int port   = event->data.addr.port;
                        subscribe_to_port(seq_handle, local_port, client, port);
                        break;
                    }

                    case SND_SEQ_EVENT_PORT_EXIT:
                    case SND_SEQ_EVENT_CLIENT_EXIT:
                    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
                        int client = event->data.addr.client;
                        int port   = event->data.addr.port;
                        snd_seq_disconnect_from(seq_handle, local_port, client, port);
                        break;
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
