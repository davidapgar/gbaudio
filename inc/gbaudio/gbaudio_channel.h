#ifndef GBAUDIO_CHANNEL_H
#define GBAUDIO_CHANNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <gbaudio/gbaudio_clock.h>


typedef enum {
    wave_duty_12 = 0x00, // _-------_-------
    wave_duty_25 = 0x01, // __------__------
    wave_duty_50 = 0x02, // ____----____----
    wave_duty_75 = 0x03, // ______--______--
} wave_duty_t;

// Base CPU clock is 4,194,304 4194304
// Frequency is 131072/(2048-x)
// Base Frequency is CPU clock / 32
// Clock divider of 32 (2^5)
// 4194304 / 32 = 131072
// Timer clocks 8 times per frequency period
// Thus, frequency/duty clock is 1,048,576 Hz (same as CPU instr cycle)
// Actual sound generation on GB is at sample rate of 1048576

// Sequence counter is clocked at 512Hz (clock/8192) (2^13)
// Sweep timer frequency 128Hz
// Length Counter at 256Hz, counting 63-0
// Volume Envelope at 64Hz

// Sound channel 1 & 2:
//                       /------------\
//                       |Master Clock|
//                       \------------/
//                             | div 4
//              div 2048       V
// /--------------\ | /-------------------\
// |Sequence 512Hz |<--|APU clock 1Mhz (/4)|
// \--------------/   \-------------------/
//   |        |  |          |
//   |        |  \-----------------\
//   V 4      |             |      V 8
// /-----\    |  2  /-----\ |   /-----\
// |Sweep|    \---->| Len | |   | Vol |
// |128Hz|     /----|256Hz|-/   | 64Hz|
// \-----/     |    \-----/     \-----/
//   |         |       |           |
//   V         V       V           V
// /-----\F /-----\S /------\S?/--------\S?/-------\
// |Sweep|->|Wave |->|Length|->|Envelope|->|DAC/Mix|
// | 0-7 |  | 0-8 |  | 0-64 |  |   0-7  |  |       |
// \-----/  \-----/  \------/  \--------/  \-------/


typedef struct gbaudio_channel_s {
    bool running;

    // Clock Dividers
    gbaudio_clock_t apu_clock;
    gbaudio_clock_t seq_clock;
    gbaudio_clock_t sweep_clock;
    gbaudio_clock_t length_clock;
    gbaudio_clock_t envelope_clock;

    /// Current sweep count
    bool sweep_enabled;
    int sweep_count;
    /// Current length count (counts down?)
    int length_count;
    /// Current envelope count
    int envelope_count;

    /// phase count, same tick rate as APU clock
    /// When it hits frequency ticks duty_count
    int phase_count;
    /// Which phase of the duty cycle (0-7) we are in.
    int duty_count;

    /// Current amplitude (controlled by envelope)
    uint8_t amplitude;

    // Running frequency in Hz.
    uint32_t frequency;
    // Amplitude to scale output signal.
    uint16_t scale_amplitude;

    // Sweep
    uint8_t sweep_time;
    bool sweep_addition;
    uint8_t sweep_shift;

    // Length/Duty Cycle
    uint8_t length;
    wave_duty_t duty;

    // Volume Envelope
    uint8_t envelope_initial;
    bool envelope_increase;
    uint8_t n_envelope;

    // Counter/Continuous (repeat mode)
    bool repeat;

    // Frequency
    uint16_t gbfreq;
} gbaudio_channel_t;

/// Convert to/from gameboy frequency
/// freq Hz = 131072/(2048 - gbfreq)
uint32_t gbfreq_to_freq(uint16_t gbfreq);
uint16_t freq_to_gbfreq(uint32_t freq);

/// Initialize a gbaudio channel.
/// Starts out with no frequency, not running.
void gbaudio_channel_init(gbaudio_channel_t *channel);

/// Set the amplitude to scale the output.
/// amplitude: 0-32768
void gbaudio_channel_set_amplitude(gbaudio_channel_t *channel, uint16_t amplitude);

/// Return the next sample for `sample_rate` frequency scaled by amplitude
int16_t gbaudio_channel_next(gbaudio_channel_t *channel, int sample_rate);

/// Return the next sample as 5-bit sample
/// Returns: -15...15, with a DC offset applied
int8_t gbaudio_channel_raw_next(gbaudio_channel_t *channel, int sample_rate);

/// Fill `n_samples` from the audio channel.
void gbaudio_channel_fill(gbaudio_channel_t *channel, int sample_rate, int16_t *samples, int n_samples);

/// Return a normalized sample as next APU tick (1Mhz)
int8_t gbaudio_channel_tick(gbaudio_channel_t *channel);

/// Apply a sweep to this channel.
/// time: 0-7, time/128Hz - time at each frequency
/// addition: increase/decrease frequency
/// shift: 0-7, Size of sweep shift
void gbaudio_channel_sweep(gbaudio_channel_t *channel, uint8_t time, bool addition, uint8_t shift);

/// Set the length and duty pattern
/// length: 0-63 - length of sound (64-length) / 256 seconds
/// duty: 12.5%, 25%, 50%, or 75% duty cycle
void gbaudio_channel_length_duty(gbaudio_channel_t *channel, uint8_t length, wave_duty_t duty);

/// Set the volume envelope
/// initial: 0-15, Initial volume (0 = No sound)
/// increase: Direction of envelope, increasing or decreasing
/// n_envelope: 0-7 Number of envelope sweeps (0 = stop)
void gbaudio_channel_volume_envelope(gbaudio_channel_t *channel, uint8_t initial, bool increase, uint8_t n_envelope);

/// "gameboy" frequency is 131072/(2048-gbfreq) Hz
/// Set the low 8 bits of "gameboy" frequency.
void gbaudio_channel_gbfreq_low(gbaudio_channel_t *channel, uint8_t freq_low);

/// Set the high 3 bits of the "gameboy" frequency.
/// high_freq: 0-7 - bits 8-10 of the frequency
void gbaudio_channel_gbfreq_high(gbaudio_channel_t *channel, uint8_t freq_high);

/// Set the 11 bit "gameboy" frequency. Calls low/high.
/// freq: 11 bits (0-2048), frequency is 131072/(2048-freq)Hz
void gbaudio_channel_gbfreq(gbaudio_channel_t *channel, uint16_t freq);

/// Set the frequency in Hz.
void gbaudio_channel_freq(gbaudio_channel_t *channel, uint32_t freq);

/// Trigger the sound channel and repeat
/// trigger: 1 = restart sound
/// single: 1 = stop after length expires, otherwise repeat.
void gbaudio_channel_trigger(gbaudio_channel_t *channel, bool trigger, bool single);

/// Trigger the sound channel, and set the high frequency bits
/// Modeled after the actual NF14 register.
void gbaudio_channel_trigger_freq_high(gbaudio_channel_t *channel, bool trigger, bool single, uint8_t freq_high);

#endif
