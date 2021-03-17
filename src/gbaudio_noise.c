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

int8_t gbaudio_noise_tick(gbaudio_noise_t *noise)
{
    return 0;
}

void gbaudio_noise_length(gbaudio_noise_t *noise, uint8_t length)
{
}

void gbaudio_noise_volume_envelope(gbaudio_noise_t *noise, uint8_t initial, bool increase, uint8_t n_envelope)
{
}

void gbaudio_noise_polynomial_counter(gbaudio_noise_t *noise, uint8_t shift_clock, bool small_step, uint8_t prescale)
{
}

void gbaudio_noise_trigger(gbaudio_noise_t *noise, bool trigger, bool single)
{
}
