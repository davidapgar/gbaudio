#ifndef LFSR_GEN_H
#define LFSR_GEN_H

#include <stdbool.h>
#include <stdint.h>

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

#endif