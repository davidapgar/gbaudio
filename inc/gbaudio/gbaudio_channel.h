#ifndef GBAUDIO_CHANNEL_H
#define GBAUDIO_CHANNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <gbaudio/audio_gen.h>


typedef enum {
    wave_duty_12 = 0x00,
    wave_duty_25 = 0x01,
    wave_duty_50 = 0x02,
    wave_duty_75 = 0x03,
} wave_duty_t;

typedef struct gbaudio_channel_s {
    bool running;
    int tick;

    // Sweep
    uint8_t sweep_time;
    bool sweep_addition;
    uint8_t n_sweep;

    // Length/Duty Cycle
    uint8_t length;
    wave_duty_t duty;

    // Volume Envelope
    uint8_t envelope_initial;
    bool envelope_increase;
    uint8_t n_envelope;

    // Frequency
    uint16_t gbfreq;
    uint32_t frequency;
} gbaudio_channel_t;

/// Convert to/from gameboy frequency
/// freq Hz = 131072/(2048 - gbfreq)
uint32_t gbfreq_to_freq(uint16_t gbfreq);
uint16_t freq_to_gbfreq(uint32_t freq);

/// Initialize a gbaudio channel.
/// Starts out with no frequency, not running.
void gbaudio_channel_init(gbaudio_channel_t *channel);

/// Return the next sample for `sample_rate` frequency.
int16_t gbaudio_channel_next(gbaudio_channel_t *channel, int sample_rate);

/// Fill `n_samples` from the audio channel.
void gbaudio_channel_fill(gbaudio_channel_t *channel, int sample_rate, int16_t *samples, int n_samples);

/// Apply a sweep to this channel.
/// time: 0-7, time/128Hz - time at each frequency
/// addition: increase/decrease frequency
/// n_shift: 0-7, Number of sweep shifts
void gbaudio_channel_sweep(gbaudio_channel_t *channel, uint8_t time, bool addition, uint8_t n_shift);

/// Set the length and duty pattern
/// length: 0-63 - length of sound (64-length) / 256
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
