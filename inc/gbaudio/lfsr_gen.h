#ifndef LFSR_GEN_H
#define LFSR_GEN_H

#include <stdbool.h>
#include <stdint.h>

#include <gbaudio/audio_gen.h>

// 15-bit lfsr generator
// XOR bits 0, 1, placed in bit 15.
// Also placed in bit 6 after shift if width is true.
typedef struct lfsr_gen_s {
    int amplitude;
    bool width;
    int update_period;

    int tick;
    uint16_t reg;
    bool last;
} lfsr_gen_t;

void lfsr_gen_init(lfsr_gen_t *lfsr, int amplitude, bool width, int update_period);
int16_t lfsr_gen_next(lfsr_gen_t *lfsr, int frequency);

void lfsr_gen_adjust_amplitude(lfsr_gen_t *lfsr, int amp);
void lfsr_gen_adjust_period(lfsr_gen_t *lfsr, int per);
void lfsr_gen_cycle_width(lfsr_gen_t *lfsr);

audio_gen_t lfsr_to_audio_gen(lfsr_gen_t *lfsr);

#endif
