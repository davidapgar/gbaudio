#include <gbaudio/freq_mod.h>


/// Amplitude of the generator underlying the modulator
/// Used to generate a range of -1...1
static int const scale = 16384;

void freq_mod_init(freq_mod_t *freq_mod, audio_gen_t *carrier, audio_gen_t *modulator)
{
    freq_mod->carrier = carrier;

    int amp = audio_gen_get_amplitude(modulator);
    audio_gen_adjust_amplitude(modulator, (scale/2) - amp);

    delta_gen_init(&freq_mod->modulator, modulator);
    freq_mod->mod_a = delta_to_audio_gen(&freq_mod->modulator);
}

int16_t freq_mod_next(freq_mod_t *freq_mod, int frequency)
{
    int16_t delta = delta_gen_next(&freq_mod->modulator, frequency);
    if (delta) {
        int freq = audio_gen_get_frequency(freq_mod->carrier);
        int change = freq * delta / scale;
        audio_gen_adjust_frequency(freq_mod->carrier, change);
    }
    return audio_gen_next(freq_mod->carrier, frequency);
}

static int16_t gen_next(void *generator, int frequency)
{
    freq_mod_t *self = (freq_mod_t *)generator;
    return freq_mod_next(self, frequency);
}

static void adjust_freq(void *generator, int freq)
{
    freq_mod_t *self = (freq_mod_t *)generator;
    audio_gen_adjust_frequency(&self->mod_a, freq);
}

static int get_freq(void *generator)
{
    freq_mod_t *self = (freq_mod_t *)generator;
    return audio_gen_get_frequency(&self->mod_a);
}

static void adjust_amp(void *generator, int amp)
{
    freq_mod_t *self = (freq_mod_t *)generator;
    audio_gen_adjust_amplitude(self->carrier, amp);
}

static int get_amp(void *generator)
{
    freq_mod_t *self = (freq_mod_t *)generator;
    return audio_gen_get_amplitude(self->carrier);
}

audio_gen_t freq_mod_to_audio_gen(freq_mod_t *freq_mod)
{
    audio_gen_t ret = {
        .generator = freq_mod,
        .next = gen_next,
        .adjust_amplitude = adjust_amp,
        .get_amplitude = get_amp,
        .adjust_frequency = adjust_freq,
        .get_frequency = get_freq,
    };
    return ret;
}
