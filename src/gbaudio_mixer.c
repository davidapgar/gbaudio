#include <gbaudio/gbaudio_mixer.h>


void gbaudio_mixer_init(gbaudio_mixer_t *mixer)
{
    mixer->enabled = false;
    gbaudio_channel_init(&mixer->channel1);
    gbaudio_channel_init(&mixer->channel2);
}

gbaudio_mixer_t gbaudio_mixer()
{
    gbaudio_mixer_t mixer;
    gbaudio_mixer_init(&mixer);

    return mixer;
}

int8_t gbaudio_mixer_tick()
{
    return 0;
}
