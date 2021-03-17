#include <gbaudio/gbaudio_mixer.h>

#include <string.h>


void gbaudio_mixer_init(gbaudio_mixer_t *mixer)
{
    memset(mixer, 0, sizeof(*mixer));

    mixer->enabled = false;
    gbaudio_channel_init(&mixer->ch1);
    gbaudio_channel_init(&mixer->ch2);
    gbaudio_noise_init(&mixer->ch4);
}

gbaudio_mixer_t gbaudio_mixer()
{
    gbaudio_mixer_t mixer;
    gbaudio_mixer_init(&mixer);

    return mixer;
}

rl_audio_t gbaudio_mixer_tick(gbaudio_mixer_t *mixer)
{
    if (!mixer->enabled) {
        rl_audio_t ret = {
            .right = 0,
            .left = 0,
        };
        return ret;
    }

    int8_t ch1_mono = gbaudio_channel_tick(&mixer->ch1);
    int8_t ch2_mono = gbaudio_channel_tick(&mixer->ch2);
    int8_t ch3_mono = 0;
    int8_t ch4_mono = gbaudio_noise_tick(&mixer->ch4);

    int8_t ch1_right = (mixer->ch1_output & output_terminal_right) ? ch1_mono : 0;
    int8_t ch1_left = (mixer->ch1_output & output_terminal_left) ? ch1_mono : 0;

    int8_t ch2_right = (mixer->ch2_output & output_terminal_right) ? ch2_mono : 0;
    int8_t ch2_left = (mixer->ch2_output & output_terminal_left) ? ch2_mono : 0;

    int8_t ch3_right = (mixer->ch3_output & output_terminal_right) ? ch3_mono : 0;
    int8_t ch3_left = (mixer->ch3_output & output_terminal_left) ? ch3_mono : 0;

    int8_t ch4_right = (mixer->ch4_output & output_terminal_right) ? ch4_mono : 0;
    int8_t ch4_left = (mixer->ch4_output & output_terminal_left) ? ch4_mono : 0;

    rl_audio_t ret = {
        .right = ch1_right + ch2_right + ch3_right + ch4_right,
        .left = ch1_left + ch2_left + ch3_left + ch4_left,
    };

    uint8_t vol_r = mixer->volume_right + 1;
    uint8_t vol_l = mixer->volume_left + 1;

    ret.right *= vol_r;
    ret.left *= vol_l;
    return ret;
}

void gbaudio_mixer_enable(gbaudio_mixer_t *mixer, bool enable)
{
    mixer->enabled = enable;
    // TODO: Reset all underlying channels if disabled
}

void gbaudio_mixer_set_output(gbaudio_mixer_t *mixer, output_terminal_t ch1_output, output_terminal_t ch2_output, output_terminal_t ch3_output, output_terminal_t ch4_output)
{
    mixer->ch1_output = ch1_output;
    mixer->ch2_output = ch2_output;
    mixer->ch3_output = ch3_output;
    mixer->ch4_output = ch4_output;
}

void gbaudio_mixer_set_volume(gbaudio_mixer_t *mixer, uint8_t right, uint8_t left)
{
    mixer->volume_right = right & 0x07;
    mixer->volume_left = left & 0x07;
}

int16_t gbaudio_mixer_mono(gbaudio_mixer_t *mixer)
{
    rl_audio_t stereo = gbaudio_mixer_tick(mixer);
    int16_t mono = (stereo.right + stereo.left) / 2;
    return mono;
}

int16_t gbaudio_mixer_next(gbaudio_mixer_t *mixer, int sample_rate)
{
    int period = (1<<20) / sample_rate;

    int16_t sample = 0;

    while (period) {
        sample = gbaudio_mixer_mono(mixer);
        --period;
    }

    int16_t scaled = ((int32_t)sample * mixer->scale_amplitude) / mixer_max;
    return scaled;
}
