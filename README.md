# Raspberry Pi Synth
A software/hardware synthesizer built on the Raspberry Pi. This project aims to provide a simple, low-latency synthesizer environment that can be used for learning audio programming, making music, or experimenting with embedded system audio development.

## Overview
This software synthesizer is designed to run on embedded Linux, specifically on the Raspberry Pi. It leverages the [Lightweight Embedded Audio Framework (LEAF)](https://github.com/spiricom/LEAF) for digital signal processing (DSP), allowing it to generate various waveforms and apply effects. A dedicated Synth API layer sits on top of LEAF to fully utilize its capabilities. MIDI input is captured via ALSA, which in turn updates and triggers the synthesizer’s sound generation. Audio output is handled by PortAudio, sending processed samples to the audio buffer for playback.

## Features

### Implemented
- Polyphonic sine oscillators, preloaded for immediate playback.
- MIDI input handling for real-time note triggering.
- Synth API for managing oscillator voices.
- Low-latency performance using ALSA for MIDI.
- Audio samples routed out through PortAudio.

### Planned
- Additional oscillator waveforms (square, saw, triangle, etc.).
- Expanded effects (reverb, delay) and adjustable effect parameters.
- Support for effect chains and more advanced DSP modules.
- MIDI clip recording and playback functionality.

## Hardware

- Raspberry Pi 2 or higher (Raspberry Pi 4 recommended for best performance).
- SD card with at least 32 GB capacity (larger recommended if storing samples or MIDI clips).
- USB Midi Controller.
- Further hardware details (optional DACs, etc.) to be added.

## Software

- [ALSA](https://www.alsa-project.org/wiki/Main_Page) for MIDI input.
- [PortAudio](https://www.portaudio.com/) for audio output.
- Build tools (e.g., make, cmake) as needed.

## Usage & Configuration
Information regarding setup, command-line arguments, configuration files, and runtime parameters will be provided in a future update.
