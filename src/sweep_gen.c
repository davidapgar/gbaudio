#include <gbaudio/sweep_gen.h>

#include <stddef.h>

void sweep_gen_init(sweep_gen_t *sweep_gen, audio_gen_t *audio_gen, bool change, int time, int n_sweep, int shift)
{
    sweep_gen->audio_gen = audio_gen;
    sweep_gen->change = change;
    if (time > 7) {
        time = 7;
    } else if (time < 0) {
        time = 0;
    }
    sweep_gen->time = time;
    if (n_sweep > 7) {
        n_sweep = 7;
    } else if (n_sweep < 0) {
        n_sweep = 0;
    }
    sweep_gen->n_sweep = n_sweep;
    if (shift > 7) {
        shift = 7;
    } else if (shift < 0) {
        shift = 0;
    }
    sweep_gen->shift = shift;

    sweep_gen->frequency = 128;
    sweep_gen->tick = 0;
}

void sweep_gen_reset(sweep_gen_t *sweep_gen)
{
    sweep_gen->tick = 0;
}

int16_t sweep_gen_next(sweep_gen_t *sweep_gen, int frequency)
{
    int period = frequency / sweep_gen->frequency;

    int tick = sweep_gen->tick + 1;

    // How long a single part of the sweep is, in ticks.
    int sweep_period = period * sweep_gen->time;

    // Total time of the sweep
    int total_time = sweep_period * sweep_gen->n_sweep;

    int16_t ret;
    if (tick < total_time) {
        if (tick % sweep_period == 0) {
            // Adjust the frequency.
            // X(t) = X(t-1) +/- X(t-1)/2^n
            int prev_freq = audio_gen_get_frequency(sweep_gen->audio_gen);
            int change = prev_freq >> sweep_gen->shift;
            change = sweep_gen->change ? change : -change;
            audio_gen_adjust_frequency(sweep_gen->audio_gen, change);
        }
        ret = audio_gen_next(sweep_gen->audio_gen, frequency);
    } else {
        ret = 0;
    }

    sweep_gen->tick = tick;
    return ret;
}

static int16_t audio_sweep_gen_next(void *generator, int freq)
{
    sweep_gen_t *sweep_gen = (sweep_gen_t *)generator;
    return sweep_gen_next(sweep_gen, freq);
}

static void audio_sweep_adjust_amplitude(void *generator, int amp)
{
    sweep_gen_t *sweep_gen = (sweep_gen_t *)generator;
    audio_gen_adjust_amplitude(sweep_gen->audio_gen, amp);
}

static int audio_sweep_get_amplitude(void *generator)
{
    sweep_gen_t *sweep_gen = (sweep_gen_t *)generator;
    return audio_gen_get_amplitude(sweep_gen->audio_gen);
}

static void audio_sweep_adjust_frequency(void *generator, int freq)
{
    sweep_gen_t *sweep_gen = (sweep_gen_t *)generator;
    audio_gen_adjust_frequency(sweep_gen->audio_gen, freq);
}

static int audio_sweep_get_frequency(void *generator)
{
    sweep_gen_t *sweep_gen = (sweep_gen_t *)generator;
    return audio_gen_get_frequency(sweep_gen->audio_gen);
}

audio_gen_t sweep_to_audio_gen(sweep_gen_t *sweep_gen)
{
    audio_gen_t ret = {
        .generator = sweep_gen,
        .next = audio_sweep_gen_next,
        .adjust_amplitude = audio_sweep_adjust_amplitude,
        .get_amplitude = audio_sweep_get_amplitude,
        .adjust_frequency = audio_sweep_adjust_frequency,
        .get_frequency = audio_sweep_get_frequency,
    };
    return ret;
}
