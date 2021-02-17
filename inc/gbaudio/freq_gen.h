#ifndef FREQ_GEN_H
#define FREQ_GEN_H

#include <stdint.h>


typedef enum {
    duty_12 = 0,
    duty_25,
    duty_50,
    duty_75,
} duty_cycle_t;

typedef struct freq_gen_s {
    int amplitude;
    int note_hz;
    duty_cycle_t duty;
    int ticks;
} freq_gen_t;

void freq_gen_init(freq_gen_t *gen, int amplitude, int frequency, duty_cycle_t duty);

int16_t freq_gen_next(freq_gen_t *gen, int sample_freq);

#endif
