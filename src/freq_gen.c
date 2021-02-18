#include <gbaudio/freq_gen.h>

#include <stddef.h>


void freq_gen_init(freq_gen_t *gen, int amplitude, int frequency, duty_cycle_t duty)
{
    // TODO: Clamp these to reasonable values?
    gen->amplitude = amplitude;
    gen->frequency = frequency;
    gen->duty = duty;
    gen->ticks = 0;
}

int16_t freq_gen_next(freq_gen_t *gen, int sample_freq)
{
    int period = sample_freq / gen->frequency;
    int ticks = gen->ticks;
    if (ticks >= period) {
        ticks -= period;
    }
    int16_t ret;

    int duty = period / 8;
    switch (gen->duty) {
    case duty_12:
        // 12.5% down duty cycle
        break;
    case duty_25:
        // 25%
        duty = duty * 2;
        break;
    case duty_50:
        // 50%
        duty = duty * 4;
        break;
    case duty_75:
        // 75%
        duty = duty * 6;
        break;
    }
    if (ticks < duty) {
        ret = gen->amplitude * -1;
    } else {
        ret = gen->amplitude;
    }
    gen->ticks = (ticks + 1);
    return ret;
}

void freq_gen_adjust_amplitude(freq_gen_t *gen, int amp_chg)
{
    int amp = gen->amplitude += amp_chg;
    if (amp > 32768) {
        amp = 32768;
    } else if (amp < 0) {
        amp = 0;
    }
    gen->amplitude = amp;
}

void freq_gen_adjust_frequency(freq_gen_t *gen, int freq_chg)
{
    int freq = gen->frequency + freq_chg;
    if (freq > 880) {
        freq = 880;
    } else if (freq < 220) {
        freq = 220;
    }
    gen->frequency = freq;
}

void freq_gen_cycle_duty(freq_gen_t *gen)
{
    duty_cycle_t duty = gen->duty;
    duty += 1;
    if (duty > duty_75) {
        duty = duty_12;
    }
    gen->duty = duty;
}

static int16_t audio_freq_gen_next(void *generator, int freq)
{
    freq_gen_t *freq_gen = (freq_gen_t *)generator;
    return freq_gen_next(freq_gen, freq);
}

static void audio_freq_adjust_amplitude(void *generator, int amp)
{
    freq_gen_t *freq_gen = (freq_gen_t *)generator;
    freq_gen_adjust_amplitude(freq_gen, amp);
}

static int audio_freq_get_amplitude(void *generator)
{
    freq_gen_t *freq_gen = (freq_gen_t *)generator;
    return freq_gen->amplitude;
}

static void audio_freq_adjust_frequency(void *generator, int freq)
{
    freq_gen_t *freq_gen = (freq_gen_t *)generator;
    freq_gen_adjust_frequency(freq_gen, freq);
}

static int audio_freq_get_frequency(void *generator)
{
    freq_gen_t *freq_gen = (freq_gen_t *)generator;
    return freq_gen->frequency;
}

audio_gen_t freq_to_audio_gen(freq_gen_t *freq_gen)
{
    audio_gen_t ret = {
        .generator = freq_gen,
        .next = audio_freq_gen_next,
        .adjust_amplitude = audio_freq_adjust_amplitude,
        .get_amplitude = audio_freq_get_amplitude,
        .adjust_frequency = audio_freq_adjust_frequency,
        .get_frequency = audio_freq_get_frequency,
    };
    return ret;
}
