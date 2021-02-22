#ifndef GBAUDIO_TO_GEN_H
#define GBAUDIO_TO_GEN_H

#include <gbaudio/audio_gen.h>
#include <gbaudio/gbaudio_channel.h>
#include <gbaudio/gbaudio_mixer.h>

audio_gen_t mixer_to_audio_gen(gbaudio_mixer_t *mixer, int amplitude);
audio_gen_t channel_to_audio_gen(gbaudio_channel_t *channel);

#endif
