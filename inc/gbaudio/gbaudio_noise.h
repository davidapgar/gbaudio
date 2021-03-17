#ifndef GBAUDIO_NOISE_H
#define GBAUDIO_NOISE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <gbaudio/gbaudio_clock.h>

// Noise, Channel 4
// Master clock to APU clock handled by APU/mixer
// Sequence/Length/Volume Envelope are same as channels
// LFSR
// Prescale clock is a simple divider (/(1-8)
// The divider is by 2 to 2^14
// (2^15 and 2^16 are "invalid")
//                       /------------\
//                       |Master Clock|
//                       \------------/
//                             | div 4
//                             V
// |---------------|   |-------------------|
// |Prescale Clock |<--|APU clock 1Mhz (/4)|
// |     (1-8)     |   |-------------------|
// |---------------|           |
//         |                   |
//         V                   | div 2048
// |---------------|           V
// |    Divider    |     |--------------|
// |   (1<<div)    |     |Sequence 512Hz|
// |---------------|     |--------------|
//        |               |         |
//        |               V 2       V 8
//        |          |-----|     |-----|
//        |          | Len |     | Vol |
//        |          |256Hz|     | 64Hz|
//        |          |-----|     |-----|
//        |             |           |
//        V             V           V
//    |------|S    |------|S?|--------|S?|-------|
//    | LFSR |---->|Length|->|Envelope|->|DAC/Mix|
//    |      |     | 0-64 |  |   0-7  |  |       |
//    |------|     |------|  |--------|  |-------|
typedef struct gbaudio_noise_s {
    bool running;

    /// Clock Dividers
    gbaudio_clock_t seq_clock;
    gbaudio_clock_t length_clock;
    gbaudio_clock_t envelope_clock;

    /// Prescale divider and shift clock divider
    int prescale_count;
    int shift_clock_count;
    /// Current length count (counts down?)
    int length_count;
    /// Current envelope count
    int envelope_count;

    /// Current amplitude (controlled by envelope)
    uint8_t amplitude;

    /// Prescale divider
    uint8_t prescale;
    /// Shift clock divider (as 1<<X)
    uint8_t shift_clock;

    /// Length
    uint8_t length;

    /// Volume Envelope
    uint8_t envelope_initial;
    bool envelope_increase;
    uint8_t n_envelope;

    /// Counter/Continuous (repeat mode)
    bool repeat;

    /// Counter width of 15 (false) or 7 (true)
    bool small_step;
    /// Actual LFSR state
    uint16_t lfsr;
    /// Last bit shifted from LFSR, to generate voltage
    uint8_t last;
} gbaudio_noise_t;

void gbaudio_noise_init(gbaudio_noise_t *noise);

/// Return a normalized sample for the next APU tick (1Mhz)
/// As a 5-bit sample
/// Returns: -15...15, with a DC offset applied.
int8_t gbaudio_noise_tick(gbaudio_noise_t *noise);

/// Set the length
/// length: 0-63, sound length is (64-length) / 256 seconds
void gbaudio_noise_length(gbaudio_noise_t *noise, uint8_t length);

/// Set the volume envelope
/// initial: 0-15, Initial volume (0 = No sound)
/// increase: Direction of envelope, increasing or decreasing
/// n_envelope: 0-7 Length of envelope sweeps (0 = stop)
/// len = n_envelope / 64 seconds
void gbaudio_noise_volume_envelope(gbaudio_noise_t *noise, uint8_t initial, bool increase, uint8_t n_envelope);

/// Set the frequency of the LFSR
/// shift_clock: 0-14, frequency divider of (1<<shift_clock)
/// small_step: set LFSR width to 15 (false) or 7 (true)
/// prescale: 0-7 Prescale divider from 1Mhz
void gbaudio_noise_polynomial_counter(gbaudio_noise_t *noise, uint8_t shift_clock, bool small_step, uint8_t prescale);

/// Trigger the noise channel
/// trigger: restart/start the noise channel
/// single: If true, will stop once length expires
void gbaudio_noise_trigger(gbaudio_noise_t *noise, bool trigger, bool single);

#endif
