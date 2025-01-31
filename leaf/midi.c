#include <alsa/asoundlib.h>

int main(){

    // open sequencer
    snd_seq_t *seq_handle;
    int err;
    err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
    if (err < 0)
    {
        printf("Error opening sequencer.");
        return 1;
    }
    snd_seq_set_client_name(seq_handle, "BSynth");

    // create port
    int in_port = snd_seq_create_simple_port(seq_handle, "Input Port",
    SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
    SND_SEQ_PORT_TYPE_MIDI_GENERIC);

    if (in_port < 0) 
    {
        printf("Error creating sequencer port: %s\n", snd_strerror(in_port));
        snd_seq_close(seq_handle);
        return 1;
    }

    printf("Our client ID is %d, port %d\n", snd_seq_client_id(seq_handle), in_port);

    //     // Enumerate all clients and ports
    // snd_seq_client_info_t *cinfo;
    // snd_seq_port_info_t   *pinfo;
    // snd_seq_client_info_alloca(&cinfo);
    // snd_seq_port_info_alloca(&pinfo);

    // // Start with the first client
    // snd_seq_client_info_set_client(cinfo, -1);

    // while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
    //     int client = snd_seq_client_info_get_client(cinfo);

    //     // Skip ourselves
    //     if (client == snd_seq_client_id(seq_handle)) {
    //         continue;
    //     }

    //     // Now query ports for this client
    //     snd_seq_port_info_set_port(pinfo, -1);
    //     while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
    //         unsigned int portCap = snd_seq_port_info_get_capability(pinfo);

    //         // // We want to find ports that can *send* MIDI data (to us),
    //         // // i.e., ports with the CAP_WRITE or CAP_SUBS_WRITE bits set
    //         // // from our perspective as an input.
    //         // if ((portCap & SND_SEQ_PORT_CAP_READ)  ||
    //         //     (portCap & SND_SEQ_PORT_CAP_SUBS_READ)) {
    //         //     // This means from the port’s perspective it can read,
    //         //     // but we want to see if it can write to us. 
    //         //     // Actually, the naming can be confusing:
    //         //     //
    //         //     //  - A "read" capability means the port can read events
    //         //     //    from the queue, i.e. it's an input in ALSA terms.
    //         //     //  - A "write" capability means it can send events out
    //         //     //    to the queue, i.e. it's an output in ALSA terms.
    //         //     //
    //         //     // So if we want to *receive* events from that port,
    //         //     // we want that port to have 'output' capability.
    //         //     // That typically is: SND_SEQ_PORT_CAP_READ or SUBS_READ 
    //         //     // from the port's perspective.
    //         //     //
    //         //     // Double-check the logic: if you see that actual devices
    //         //     // appear with CAP_WRITE, you may need to invert the logic.
    //         // }

    //         // Alternatively, you can check:
    //         if ((portCap & SND_SEQ_PORT_CAP_WRITE) ||
    //             (portCap & SND_SEQ_PORT_CAP_SUBS_WRITE)) {
    //             // Many hardware MIDI ports expose themselves as "write" 
    //             // from the device side, meaning they 'write' data into 
    //             // the ALSA sequencer. So, this is commonly how you'd detect 
    //             // a hardware MIDI OUT which can feed events to us.

    //             int remote_port = snd_seq_port_info_get_port(pinfo);

    //             // Let's connect it
    //             printf("Connecting from client %d, port %d (%s)\n",
    //                    client, remote_port,
    //                    snd_seq_port_info_get_name(pinfo));

    //             snd_seq_port_subscribe_t *subs;
    //             snd_seq_port_subscribe_alloca(&subs);

    //             // Set the addresses (from remote to our local port)
    //             snd_seq_addr_t sender, receiver;
    //             sender.client = client;
    //             sender.port   = remote_port;
    //             receiver.client = snd_seq_client_id(seq_handle);
    //             receiver.port   = in_port;

    //             snd_seq_port_subscribe_set_sender(subs, &sender);
    //             snd_seq_port_subscribe_set_dest(subs, &receiver);

    //             // Actually create the subscription
    //             err = snd_seq_subscribe_port(seq_handle, subs);
    //             if (err < 0) {
    //                 fprintf(stderr, "Could not subscribe to %d:%d - %s\n",
    //                         client, remote_port, snd_strerror(err));
    //             }
    //             else {
    //                 printf("Subscribed to %d:%d successfully.\n", client, remote_port);
    //             }
    //         }
    //     }
    // }

    // printf("Done enumerating.\n");
    // printf("Now reading events from our port in a loop.\n");

    int MIDI_CLIENT = 40;
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

    snd_seq_event_t *event;
    while (1) 
    {
        snd_seq_event_t *event = NULL;
        // This call blocks until an event is available
        // unless you open the queue in non-blocking mode.
        if (snd_seq_event_input(seq_handle, &event) >= 0) {
            // Handle the event
            if (event) {
                if (event->type == SND_SEQ_EVENT_NOTEON) {
                    printf("Note On: chan=%d note=%d vel=%d\n",
                           event->data.note.channel,
                           event->data.note.note,
                           event->data.note.velocity);
                }
                else if (event->type == SND_SEQ_EVENT_NOTEOFF) {
                    printf("Note Off: chan=%d note=%d vel=%d\n",
                           event->data.note.channel,
                           event->data.note.note,
                           event->data.note.velocity);
                } else if (event->type == SND_SEQ_EVENT_CONTROLLER) {
                    printf("Control Change: channel=%d controller=%d value=%d\n",
                        event->data.control.channel,
                        event->data.control.param,
                        event->data.control.value);
                    // Do something with the CC data, e.g. update a parameter or log it
                }
                // add more event handling as needed...
                snd_seq_free_event(event);
            }
        }
    }
}