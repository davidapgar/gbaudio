#include <gbaudio/gbaudio_channel.h>

#include <string.h>


uint32_t gbfreq_to_freq(uint16_t gbfreq)
{
    if (gbfreq > 2047) {
        gbfreq = 2047;
    }
    uint32_t freq = 131072 / (2048 - gbfreq);

    return freq;
}

uint16_t freq_to_gbfreq(uint32_t freq)
{
    if (freq == 0) {
        freq = 1;
    }

    uint16_t gbfreq = 2048 - (131072 / freq);

    return gbfreq;
}

void gbaudio_channel_init(gbaudio_channel_t *channel)
{
    memset(channel, 0, sizeof(*channel));
}

int16_t gbaudio_channel_next(gbaudio_channel_t *channel, int sample_rate)
{
    return 0;
}

void gbaudio_channel_fill(gbaudio_channel_t *channel, int sample_rate, int16_t *samples, int n_samples)
{
    for (int i = 0; i < n_samples; ++i) {
        samples[i] = gbaudio_channel_next(channel, sample_rate);
    }
}

void gbaudio_channel_sweep(gbaudio_channel_t *channel, uint8_t time, bool addition, uint8_t n_shift)
{
    channel->sweep_time = (time & 0x07);
    channel->sweep_addition = addition;
    channel->n_sweep = (n_shift & 0x07);
}

void gbaudio_channel_length_duty(gbaudio_channel_t *channel, uint8_t length, wave_duty_t duty)
{
    channel->length = (length & 0x3F);
    channel->duty = duty;
}

void gbaudio_channel_volume_envelope(gbaudio_channel_t *channel, uint8_t initial, bool increase, uint8_t n_envelope)
{
    channel->envelope_initial = (initial & 0x0F);
    channel->envelope_increase = increase;
    channel->n_envelope = n_envelope;
}

static void update_freq(gbaudio_channel_t *channel)
{
    channel->frequency = gbfreq_to_freq(channel->gbfreq);
}

void gbaudio_channel_gbfreq_low(gbaudio_channel_t *channel, uint8_t freq_low)
{
    // Preserve high bits
    uint16_t gbfreq = (channel->gbfreq & 0x0300);
    gbfreq |= freq_low;

    channel->gbfreq = gbfreq;
    update_freq(channel);
}

void gbaudio_channel_gbfreq_high(gbaudio_channel_t *channel, uint8_t freq_high)
{
    // Preserve low bits
    uint16_t gbfreq = (channel->gbfreq & 0x00ff);
    uint16_t high = (freq_high & 0x03);
    channel->gbfreq = (high << 8) | gbfreq;

    update_freq(channel);
}

void gbaudio_channel_gbfreq(gbaudio_channel_t *channel, uint16_t freq)
{
    channel->gbfreq = (freq & 0x03ff);
    channel->gbfreq = freq;
    update_freq(channel);
}

void gbaudio_channel_freq(gbaudio_channel_t *channel, uint32_t freq)
{
    uint16_t gbfreq = freq_to_gbfreq(freq);
    gbaudio_channel_gbfreq(channel, gbfreq);
}

void gbaudio_channel_trigger(gbaudio_channel_t *channel, bool trigger, bool single)
{
}

void gbaudio_channel_trigger_freq_high(gbaudio_channel_t *channel, bool trigger, bool single, uint8_t freq_high)
{
    gbaudio_channel_gbfreq_high(channel, freq_high);
    gbaudio_channel_trigger(channel, trigger, single);
}
