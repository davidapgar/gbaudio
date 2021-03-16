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
    channel->apu_clock = gbaudio_clock(22, 20);
    channel->seq_clock = sequencer_clock();
    channel->sweep_clock = sweep_clock();
    channel->length_clock = length_clock();
    channel->envelope_clock = envelope_clock();
}

void gbaudio_channel_set_amplitude(gbaudio_channel_t *channel, uint16_t amplitude)
{
    channel->scale_amplitude = amplitude;
}

/// Return the current sample at APU clock sample frequency
/// Normalized around 0.
int8_t gbaudio_channel_sample(gbaudio_channel_t *channel)
{
    bool sample = false;
    int duty_count = channel->duty_count;

    switch (channel->duty) {
    case wave_duty_12:
        if (duty_count < 1)
            sample = true;
        break;
    case wave_duty_25:
        if (duty_count < 2)
            sample = true;
        break;
    case wave_duty_50:
        if (duty_count < 4)
            sample = true;
        break;
    case wave_duty_75:
        if (duty_count < 6)
            sample = true;
        break;
    }

    int8_t ret = 0;
    int8_t amplitude = channel->amplitude;
    if (sample) {
        ret = amplitude;
    } else {
        ret = -amplitude;
    }
    return ret;
}

/// Tick a counter by one, resetting to zero if
/// hitting the threshold
/// returns true if the threshold is hit
static int counter_tick(int *counter, int const threshold, int val)
{
    *counter += val;
    if (*counter >= threshold) {
        *counter -= threshold;
        return true;
    } else {
        return false;
    }
}

static void tick_duty(gbaudio_channel_t *channel)
{
    // (2048-gbfreq) is the phase counter
    int freq = 2048 - channel->gbfreq;

    // Phase counter
    counter_tick(&channel->duty_count, 8,
        counter_tick(&channel->phase_count, freq, true));
}

static void tick_sweep(gbaudio_channel_t *channel, uint32_t seq_tick)
{
    uint32_t sweep_tick = gbaudio_clock_step(&channel->sweep_clock, seq_tick);

    // Sweep disabled.
    if (!channel->sweep_enabled) {
        return;
    }

    if (counter_tick(
        &channel->sweep_count,
        channel->sweep_time,
        sweep_tick)) {

        // Sweep time ticked, adjust frequency
        uint16_t freq = channel->gbfreq;
        uint16_t shift = freq >> channel->sweep_shift;
        if (channel->sweep_addition) {
            freq += shift;
            if (freq > 2047) {
                // disable channel completely if overflow
                channel->sweep_enabled = false;
                channel->running = false;
                freq = channel->gbfreq;
            }
        } else { // subtraction
            // If result would be less than zero, use last value
            if (shift < freq) {
                freq -= shift;
            }
        }
        channel->gbfreq = freq;
    }

}

static void tick_length(gbaudio_channel_t *channel, uint32_t seq_tick)
{
    uint32_t length_tick = gbaudio_clock_step(&channel->length_clock, seq_tick);

    if (counter_tick(
        &channel->length_count,
        64 - channel->length,
        length_tick)) {

        // Length counter expired, repeat isn't set.
        if (channel->length_count == 0 && !channel->repeat) {
            channel->running = false;
        }
    }
}

static void tick_envelope(gbaudio_channel_t *channel, uint32_t seq_tick)
{
    uint32_t envelope_tick = gbaudio_clock_step(&channel->envelope_clock, seq_tick);

    if (!channel->n_envelope) {
        return;
    }

    if (counter_tick(
        &channel->envelope_count,
        channel->n_envelope,
        envelope_tick)) {

        uint8_t amplitude = channel->amplitude;

        if (channel->envelope_increase) {
            amplitude += (amplitude == 15 ? 0 : 1);
        } else {
            amplitude -= (amplitude == 0 ? 0 : 1);
        }
        // TODO: Stop the envelope if 15 or 0?
        channel->amplitude = amplitude;
    }
}

/// Input 4 clock tick
/// Output is scaled by 2x to balance at zero.
int8_t gbaudio_channel_tick(gbaudio_channel_t *channel)
{
    int8_t sample;
    if (channel->running) {
        sample = gbaudio_channel_sample(channel);
    } else {
        sample = 0;
    }

    uint32_t seq_tick = gbaudio_clock_step(&channel->seq_clock, 1);

    tick_duty(channel);
    tick_sweep(channel, seq_tick);
    tick_length(channel, seq_tick);
    tick_envelope(channel, seq_tick);

    return sample;
}

int16_t gbaudio_channel_next(gbaudio_channel_t *channel, int sample_rate)
{
    int8_t sample = gbaudio_channel_raw_next(channel, sample_rate);
    int16_t scaled = (channel->scale_amplitude * sample) / 32;

    return scaled;
}

int8_t gbaudio_channel_raw_next(gbaudio_channel_t *channel, int sample_rate)
{
    int period = (1<<20) / sample_rate;
    int8_t sample = 0;

    // Nearest neighbor
    while (period) {
        sample = gbaudio_channel_tick(channel);
        period--;
    }
    return sample;
}

void gbaudio_channel_fill(gbaudio_channel_t *channel, int sample_rate, int16_t *samples, int n_samples)
{
    for (int i = 0; i < n_samples; ++i) {
        samples[i] = gbaudio_channel_next(channel, sample_rate);
    }
}

void gbaudio_channel_sweep(gbaudio_channel_t *channel, uint8_t time, bool addition, uint8_t shift)
{
    channel->sweep_time = (time & 0x07);
    channel->sweep_addition = addition;
    channel->sweep_shift = (shift & 0x07);
    if (channel->sweep_time) {
        channel->sweep_enabled = true;
    }
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

    channel->amplitude = channel->envelope_initial;
}

static void update_freq(gbaudio_channel_t *channel)
{
    channel->frequency = gbfreq_to_freq(channel->gbfreq);
}

void gbaudio_channel_gbfreq_low(gbaudio_channel_t *channel, uint8_t freq_low)
{
    // Preserve high bits
    uint16_t gbfreq = (channel->gbfreq & 0x0700);
    gbfreq |= freq_low;

    channel->gbfreq = gbfreq;
    update_freq(channel);
}

void gbaudio_channel_gbfreq_high(gbaudio_channel_t *channel, uint8_t freq_high)
{
    // Preserve low bits
    uint16_t gbfreq = (channel->gbfreq & 0x00ff);
    uint16_t high = (freq_high & 0x07);
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
    if (trigger) {
        channel->running = true;
        channel->sweep_count = 0;
        channel->length_count = 0;
        channel->envelope_count = 0;
        if (channel->sweep_time) {
            channel->sweep_enabled = true;
        }
    }
    channel->repeat = !single;
}

void gbaudio_channel_trigger_freq_high(gbaudio_channel_t *channel, bool trigger, bool single, uint8_t freq_high)
{
    gbaudio_channel_gbfreq_high(channel, freq_high);
    gbaudio_channel_trigger(channel, trigger, single);
}
