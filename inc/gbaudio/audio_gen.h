#ifndef AUDIO_GEN_H
#define AUDIO_GEN_H

#include <stdint.h>

typedef struct audio_gen_s audio_gen_t;
typedef int16_t (*audio_gen_next_t)(void* generator, int frequency);
typedef void (*audio_gen_adjust_amplitude_t)(void *generator, int amp);
typedef int (*audio_gen_get_amplitude_t)(void *generator);
typedef void (*audio_gen_adjust_frequency_t)(void *generator, int freq);
typedef int (*audio_gen_get_frequency_t)(void *generator);

struct audio_gen_s {
    void *generator;
    audio_gen_next_t next;
    audio_gen_adjust_amplitude_t adjust_amplitude;
    audio_gen_get_amplitude_t get_amplitude;
    audio_gen_adjust_frequency_t adjust_frequency;
    audio_gen_get_frequency_t get_frequency;
};

int16_t audio_gen_next(audio_gen_t *audio_gen, int frequency);
void audio_gen_adjust_amplitude(audio_gen_t *audio_gen, int amp);
int audio_gen_get_amplitude(audio_gen_t *audio_gen);
void audio_gen_adjust_frequency(audio_gen_t *audio_gen, int freq);
int audio_gen_get_frequency(audio_gen_t *audio_gen);

#endif
