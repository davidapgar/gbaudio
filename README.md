SDL Audio Example
=================

This is a simple SDL audio example for generated frequencies. Built while working on a gameboy emulator to experiment with the type of sound generation needed.

## Building

Requires SDL2 and SDL2_ttf. Has only been tested on mac os, but likely should build on linux/windows.

The binary `gbaudio_demo` is built in `build/output/bin`. Requires a font to run as the first argument. The excellent open VT323 font is included in the repo.

## Replay Audio

For testing the emulator, added the ability to replay "audio" based on register writes recorded.

Format of the replay log is (in text) `Ticks reg_addr value` where ticks is a 32-bit unsigned hex of how many cpu cycles have passed (at a clock rate of 1Mhz for DMG), reg_addr should be a valid APU register ($FF10...$FF26), and value is the 8-bit value written. This drives a mixer (that currently only supports channels 1 and 2).

Also, if `raw_file` is a valid file descriptor, the audio callback will write all of the samples to disk as a raw PCM of 16-bit signed little endian samples at 44100Hz.

## References

(Random references related to the gameboy APU)

https://gbdev.io/pandocs/#sound-controller

https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
