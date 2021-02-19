#ifndef SAW_GEN_H
#define SAW_GEN_H

#include <gbaudio/audio_gen.h>

typedef struct saw_gen_s {
    int frequency;
    int amplitude;
    int ticks;
} saw_gen_t;

void saw_gen_init(saw_gen_t *gen, int amplitude, int frequency);
int16_t saw_gen_next(saw_gen_t *gen, int frequency);
audio_gen_t saw_to_audio_gen(saw_gen_t *saw);

#endif
