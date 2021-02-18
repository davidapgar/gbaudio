#include <gbaudio/audio_gen.h>


int16_t audio_gen_next(audio_gen_t *audio_gen, int frequency)
{
    if (!audio_gen->next) {
        return 0;
    }
    return audio_gen->next(audio_gen->generator, frequency);
}

void audio_gen_adjust_amplitude(audio_gen_t *audio_gen, int amp)
{
    if (!audio_gen->adjust_amplitude) {
        return;
    }
    audio_gen->adjust_amplitude(audio_gen->generator, amp);
}

int audio_gen_get_amplitude(audio_gen_t *audio_gen)
{
    if (!audio_gen->get_amplitude) {
        return 0;
    }
    return audio_gen->get_amplitude(audio_gen->generator);
}

void audio_gen_adjust_frequency(audio_gen_t *audio_gen, int freq)
{
    if (!audio_gen->adjust_frequency) {
        return;
    }
    audio_gen->adjust_frequency(audio_gen->generator, freq);
}

int audio_gen_get_frequency(audio_gen_t *audio_gen)
{
    if (!audio_gen->get_frequency) {
        return 0;
    }
    return audio_gen->get_frequency(audio_gen->generator);
}
