#ifndef GBAUDIO_MIXER_H
#define GBAUDIO_MIXER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#include <gbaudio/gbaudio_channel.h>


typedef struct gbaudio_mixer_s {
    /// Sound controller enabled/disabled
    bool enabled;

    gbaudio_channel_t channel1;
    gbaudio_channel_t channel2;
    // TODO: WAV channel and LFSR channel
} gbaudio_mixer_t;

void gbaudio_mixer_init(gbaudio_mixer_t *mixer);
gbaudio_mixer_t gbaudio_mixer();

/// Tick the mixer by one APU clock.
int8_t gbaudio_mixer_tick();

#endif
