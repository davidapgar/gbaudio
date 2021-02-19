#ifndef DELTA_GEN_H
#define DELTA_GEN_H

#include <gbaudio/audio_gen.h>

/// Returns the change in frequence of the underlying generator
typedef struct delta_gen_s {
    audio_gen_t *generator;
    int16_t last;
} delta_gen_t;

void delta_gen_init(delta_gen_t *delta, audio_gen_t *generator);
int16_t delta_gen_next(delta_gen_t *delta, int frequency);
audio_gen_t delta_to_audio_gen(delta_gen_t *delta);

#endif
