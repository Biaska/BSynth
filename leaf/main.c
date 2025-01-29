#include "leaf.h"

#define MEM_SIZE 500000
#define AUDIO_BUFFER_SIZE 128

float randomNumber()
{
    return (float)rand() / (float)RAND_MAX;
}

int main()
{
    // in your user code, create an instance of a master leaf object. This
    // exists so that in the case of a plugin environment or other situation
    // with shared resources, you can still have separate instances of the
    // LEAF library that won't conflict.
    LEAF leaf;

    // then create instances of whatever LEAF objects you want
    tCycle mySine;

    // also, create a memory pool array where you can store the data for the
    // LEAF objects. It should be an array of chars. Note that you can also
    // define multiple mempool objects in different memory locations in you want
    // to, and initialize different LEAF objects in different places. However,
    // LEAF needs at least one mempool, so the one you pass into the LEAF_init
    // function is considered the default location for any LEAF-related memory,
    // unless you specifically put something elsewhere by using an initToPool()
    // function instead of init(). LEAF object store a pointer to which mempool
    // they are in, so if you initToPool it will remember where you put it and
    // the free() function will free it from the correct location.

    char myMemory[MEM_SIZE];

    // we'll assume your code has some kind of audio buffer that is transmitting
    // samples to an audio codec or an operating system's audio driver. In this
    // example, let's define this here.

    float audioBuffer[AUDIO_BUFFER_SIZE];

    // then initialize the whole LEAF library (this only needs to be
    // done once, it sets global parameters like the default mempool and the sample rate)
    // the parameters are: master leaf instance, sample rate, audio buffer size in samples,
    // name of mempool array, size of mempool array, and address of a function to generate a
    // random number. In this case, there is a function called randomNumber that exists elsewhere
    // in the user code that generates a random floating point number from 0.0 to 1.0. We ask the
    // user to pass in a random number function because LEAF has no dependencies, and users
    // developing on embedded systems may want to use a hardware RNG, for instance.

    printf("LEAF Initialized...\n");
    LEAF_init(&leaf, 48000, myMemory, MEM_SIZE, &randomNumber);

    // now initialize the object you want to use, in this case the sine wave
    // oscillator you created above. You need to also pass in the instance of the
    // master leaf object (only needed for initializing objects).

    tCycle_init(&mySine, &leaf);

    // set the frequency of the oscillator (defaults to zero). In a real use case,
    // you'd probably want to be updating this to new values in the audio frame based
    // on knob positions or midi data or other inputs, but here we'll assume it stays
    // fixed.

    printf("HZ set to 440...\n");
    tCycle_setFreq(mySine, 440.0);

    // now, in your audio callback (a function that will be called every audio frame,
    // to compute the samples needed to fill the audio buffer) tick the LEAF audio object
    // to generate or process audio samples.

    printf("Playing sound...\n");
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        audioBuffer[i] = tCycle_tick(mySine);
    }

    return 0;
}