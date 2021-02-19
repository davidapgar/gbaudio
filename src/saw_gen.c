#include <gbaudio/saw_gen.h>

#include <stddef.h>


void saw_gen_init(saw_gen_t *gen, int amplitude, int frequency)
{
    // TODO: Clamp these to reasonable values?
    gen->amplitude = amplitude;
    gen->frequency = frequency;
    gen->ticks = 0;
}

int16_t saw_gen_next(saw_gen_t *gen, int frequency)
{
    int period = frequency / gen->frequency;
    int ticks = gen->ticks;
    if (ticks >= period) {
        ticks -= period;
    }
    int16_t ret = 0;

    // 4 phases in the period (is phase the right term?)
    // \         /
    //  \|  |  |/
    //___\__|__/
    //   |\ | /|
    // 1 |2\|/3| 4
    int phase = period / 4;
    if (ticks < phase) { // phase 1, falling positive
        int relative = ticks;
        ret = gen->amplitude * (phase - relative) / phase;
    } else if (ticks < (phase*2)) { // phase 2, falling negative
        int relative = ticks - phase;
        ret = -gen->amplitude * (relative) / phase;
    } else if (ticks < (phase*3)) { // phase 3, rising negative
        int relative = ticks - (phase*2);
        ret = -gen->amplitude * (phase - relative) / phase;
    } else { // (ticks < (phase*4)) phase 4, rising positive
        int relative = ticks - (phase*3);
        ret = gen->amplitude * (relative) / phase;
    }

    gen->ticks = (ticks + 1);
    return ret;
}

void saw_gen_adjust_amplitude(saw_gen_t *gen, int amp_chg)
{
    int amp = gen->amplitude += amp_chg;
    if (amp > 32768) {
        amp = 32768;
    } else if (amp < 0) {
        amp = 0;
    }
    gen->amplitude = amp;
}

void saw_gen_adjust_frequency(saw_gen_t *gen, int freq_chg)
{
    int freq = gen->frequency + freq_chg;
    if (freq > 880) {
        freq = 880;
    } else if (freq < 80) {
        freq = 80;
    }
    gen->frequency = freq;
}

static int16_t gen_next(void *generator, int frequency)
{
    saw_gen_t *self = (saw_gen_t *)generator;
    return saw_gen_next(self, frequency);
}

static void adjust_freq(void *generator, int freq)
{
    saw_gen_t *self = (saw_gen_t *)generator;
    saw_gen_adjust_frequency(self, freq);
}

static int get_freq(void *generator)
{
    saw_gen_t *self = (saw_gen_t *)generator;
    return self->frequency;
}

static void adjust_amp(void *generator, int amp)
{
    saw_gen_t *self = (saw_gen_t *)generator;
    saw_gen_adjust_amplitude(self, amp);
}

static int get_amp(void *generator)
{
    saw_gen_t *self = (saw_gen_t *)generator;
    return self->amplitude;
}

audio_gen_t saw_to_audio_gen(saw_gen_t *saw)
{
    audio_gen_t ret = {
        .generator = saw,
        .next = gen_next,
        .adjust_amplitude = adjust_amp,
        .get_amplitude = get_amp,
        .adjust_frequency = adjust_freq,
        .get_frequency = get_freq,
    };
    return ret;
}
