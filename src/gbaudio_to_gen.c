#include <gbaudio/gbaudio_to_gen.h>


// MARK - Channel

static int16_t ch_gen_next(void *generator, int frequency)
{
    gbaudio_channel_t *self = (gbaudio_channel_t *)generator;
    return gbaudio_channel_next(self, frequency);
}

static void ch_adjust_amp(void *generator, int amp)
{
    gbaudio_channel_t *self = (gbaudio_channel_t *)generator;
    self->scale_amplitude += amp;
}

static int ch_get_amp(void *generator)
{
    gbaudio_channel_t *self = (gbaudio_channel_t *)generator;
    return self->scale_amplitude;
}

static int ch_get_freq(void *generator)
{
    gbaudio_channel_t *self = (gbaudio_channel_t *)generator;
    return self->frequency;
}

audio_gen_t channel_to_audio_gen(gbaudio_channel_t *channel)
{
    audio_gen_t ret = {
        .generator = channel,
        .next = ch_gen_next,
        .adjust_amplitude = ch_adjust_amp,
        .get_amplitude = ch_get_amp,
        .adjust_frequency = NULL,
        .get_frequency = ch_get_freq,
    };
    return ret;
}

// MARK - Mixer

static int16_t mix_gen_next(void *generator, int frequency)
{
    gbaudio_mixer_t *self = (gbaudio_mixer_t *)generator;
    return gbaudio_mixer_next(self, frequency);
}

static void mix_adjust_amp(void *generator, int amp)
{
    gbaudio_mixer_t *self = (gbaudio_mixer_t *)generator;
    self->scale_amplitude += amp;
}

static int mix_get_amp(void *generator)
{
    gbaudio_mixer_t *self = (gbaudio_mixer_t *)generator;
    return self->scale_amplitude;
}

audio_gen_t mixer_to_audio_gen(gbaudio_mixer_t *mixer, int amplitude)
{
    mixer->scale_amplitude = amplitude;
    audio_gen_t ret = {
        .generator = mixer,
        .next = mix_gen_next,
        .adjust_amplitude = mix_adjust_amp,
        .get_amplitude = mix_get_amp,
        .adjust_frequency = NULL,
        .get_frequency = NULL,
    };
    return ret;
}
