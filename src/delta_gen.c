#include <gbaudio/delta_gen.h>


void delta_gen_init(delta_gen_t *delta, audio_gen_t *generator){
    delta->generator = generator;
    // Seed an initial value.
    delta->last = audio_gen_next(generator, 44100);
}

int16_t delta_gen_next(delta_gen_t *delta, int frequency)
{
    int16_t last = delta->last;
    int16_t next = audio_gen_next(delta->generator, frequency);
    int16_t ret = next - last;
    delta->last = next;

    return ret;
}

static int16_t gen_next(void *generator, int frequency)
{
    delta_gen_t *self = (delta_gen_t *)generator;
    return delta_gen_next(self, frequency);
}

static void adjust_freq(void *generator, int freq)
{
    delta_gen_t *self = (delta_gen_t *)generator;
    audio_gen_adjust_frequency(self->generator, freq);
}

static int get_freq(void *generator)
{
    delta_gen_t *self = (delta_gen_t *)generator;
    return audio_gen_get_frequency(self->generator);
}

static void adjust_amp(void *generator, int amp)
{
    delta_gen_t *self = (delta_gen_t *)generator;
    audio_gen_adjust_amplitude(self->generator, amp);
}

static int get_amp(void *generator)
{
    delta_gen_t *self = (delta_gen_t *)generator;
    return audio_gen_get_amplitude(self->generator);
}

audio_gen_t delta_to_audio_gen(delta_gen_t *delta)
{
    audio_gen_t ret = {
        .generator = delta,
        .next = gen_next,
        .adjust_amplitude = adjust_amp,
        .get_amplitude = get_amp,
        .adjust_frequency = adjust_freq,
        .get_frequency = get_freq,
    };
    return ret;
}
