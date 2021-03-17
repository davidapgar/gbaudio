#include <gbaudio/gbaudio_noise.h>

#include <string.h>


/// Tick a counter by one, resetting to zero if
/// hitting the threshold
/// returns true if the threshold is hit
int counter_tick(int *counter, int const threshold, int val)
{
    *counter += val;
    if (*counter >= threshold) {
        *counter = 0;
        return true;
    } else {
        return false;
    }
}

void gbaudio_noise_init(gbaudio_noise_t *noise)
{
    memset(noise, 0, sizeof(*noise));
    // LFSR *must* be initialized as non-zero.
    noise->lfsr = 1;

    // Setup all clocks
    noise->seq_clock = sequencer_clock();
    noise->length_clock = length_clock();
    noise->envelope_clock = envelope_clock();
}

int8_t gbaudio_noise_sample(gbaudio_noise_t *noise)
{
    int8_t ret = 0;
    int8_t amplitude = noise->amplitude;
    // The last bit is apparently inverted per the gbdev wiki
    if (!noise->last) {
        ret = amplitude;
    } else {
        ret = -amplitude;
    }
    return ret;
}

/// Call once per APU clock
static void tick_lfsr(gbaudio_noise_t *noise)
{
    int shift_divider = (1 << noise->shift_clock);

    if (counter_tick(
        &noise->shift_clock_count,
        shift_divider,
        counter_tick(
            &noise->prescale_count,
            noise->prescale,
            1))) {

        // Update LFSR
        // Update current output bit
        uint8_t bit = (noise->lfsr & 0x01);
        noise->last = bit;

        uint16_t lfsr = (noise->lfsr >> 1);

        bit = bit ^ (lfsr & 0x01);
        lfsr |= (bit << 14);
        if (noise->small_step) {
            lfsr |= (bit << 6);
        }
        noise->lfsr = lfsr;
    }
}

static void tick_length(gbaudio_noise_t *channel, uint32_t seq_tick)
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

static void tick_envelope(gbaudio_noise_t *channel, uint32_t seq_tick)
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
        channel->amplitude = amplitude;
    }
}

/// Called once per 4 clock ticks (single APU tick)
/// Output is scaled to 2x to balance at zero.
int8_t gbaudio_noise_tick(gbaudio_noise_t *noise)
{
    if (!noise->running) {
        return 0;
    }
    int8_t sample = gbaudio_noise_sample(noise);

    uint32_t seq_tick = gbaudio_clock_step(&noise->seq_clock, 1);
    tick_lfsr(noise);
    tick_length(noise, seq_tick);
    tick_envelope(noise, seq_tick);

    return sample;
}

void gbaudio_noise_length(gbaudio_noise_t *noise, uint8_t length)
{
    noise->length = (length & 0x3f); // 0-63
}

void gbaudio_noise_volume_envelope(gbaudio_noise_t *noise, uint8_t initial, bool increase, uint8_t n_envelope)
{
    noise->envelope_initial = (initial & 0x0f); // 0-15
    noise->envelope_increase = increase;
    noise->n_envelope = (n_envelope & 0x07); // 0-7

    noise->amplitude = noise->envelope_initial;
}

void gbaudio_noise_polynomial_counter(gbaudio_noise_t *noise, uint8_t shift_clock, bool small_step, uint8_t prescale)
{
    shift_clock = (shift_clock & 0x0f); // 0-15
    if (shift_clock >= 14) {
        shift_clock = 14;
    }
    noise->shift_clock = shift_clock;
    noise->small_step = small_step;
    noise->prescale = (prescale & 0x07); // 0-7
}

void gbaudio_noise_trigger(gbaudio_noise_t *noise, bool trigger, bool single)
{
    if (trigger) {
        noise->running = true;
        noise->prescale_count = 0;
        noise->shift_clock_count = 0;
        noise->length_count = 0;
        noise->envelope_count = 0;
        noise->lfsr = 0x01;
        noise->last = 0;
    }
    noise->repeat = !single;
}
