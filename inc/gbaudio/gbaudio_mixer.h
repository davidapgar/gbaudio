#ifndef GBAUDIO_MIXER_H
#define GBAUDIO_MIXER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#include <gbaudio/gbaudio_channel.h>

// Mixer expects 5 bit signed sound input
// Mixing is additive across all channels
// Outputs is up to 8x the input (volume 0-7 values)
// From gbdev wiki: "...at volume 2 with a master level 7 is about as loud as that channel playing at volume 15 with a master volume 0"
// Mean the possible maximum (positive) value would be 512 (9-bit)
// 16max * 4channel * 8volume = 512

typedef enum {
    output_terminal_none = 0x00,
    output_terminal_right = 0x01, // SO1
    output_terminal_left = 0x02, // SO2
    output_terminal_both = 0x03,
} output_terminal_t;

enum {
    mixer_max = 512,
};

typedef struct gbaudio_mixer_s {
    /// Sound controller enabled/disabled
    bool enabled;

    gbaudio_channel_t ch1;
    gbaudio_channel_t ch2;
    // TODO: WAV channel and LFSR channel

    output_terminal_t ch1_output;
    output_terminal_t ch2_output;

    uint8_t volume_right;
    uint8_t volume_left;

    // For PCM output, amplitude to scale output to.
    int scale_amplitude;
} gbaudio_mixer_t;

void gbaudio_mixer_init(gbaudio_mixer_t *mixer);
gbaudio_mixer_t gbaudio_mixer();

/// Tick the mixer by one APU clock.
/// Return: Next left/right audio sample mixed and scaled by master volume level.
typedef struct rl_audio_s {
    int16_t right;
    int16_t left;
} rl_audio_t;
rl_audio_t gbaudio_mixer_tick(gbaudio_mixer_t *mixer);

void gbaudio_mixer_enable(gbaudio_mixer_t *mixer, bool enable);
void gbaudio_mixer_set_output(gbaudio_mixer_t *mixer, output_terminal_t ch1_output, output_terminal_t ch2_output);
void gbaudio_mixer_set_volume(gbaudio_mixer_t *mixer, uint8_t right, uint8_t left);

/// Convenience to merge stereo output back to mono.
/// Averages the value of right and left channels.
int16_t gbaudio_mixer_mono(gbaudio_mixer_t *mixer);

/// Convenience to tick the underlying channels to generate a PCM sample at sample_rate.
int16_t gbaudio_mixer_next(gbaudio_mixer_t *mixer, int sample_rate);

#endif
