#ifndef FREQ_GEN_H
#define FREQ_GEN_H

#include <stdbool.h>
#include <stdint.h>

#include <gbaudio/audio_gen.h>


typedef enum {
    duty_12 = 0,
    duty_25,
    duty_50,
    duty_75,
} duty_cycle_t;

typedef struct freq_gen_s {
    int amplitude;
    int frequency;
    duty_cycle_t duty;
    int ticks;
} freq_gen_t;

void freq_gen_init(freq_gen_t *gen, int amplitude, int frequency, duty_cycle_t duty);

int16_t freq_gen_next(freq_gen_t *gen, int sample_freq);
/// Adjust amplitude or frequency up or down.
void freq_gen_adjust_amplitude(freq_gen_t *gen, int amp_chg);
void freq_gen_adjust_frequency(freq_gen_t *gen, int freq_chg);
/// Cycle to the next duty cycle.
void freq_gen_cycle_duty(freq_gen_t *gen);

audio_gen_t freq_to_audio_gen(freq_gen_t *freq_gen);

#endif
