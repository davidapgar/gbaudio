#ifndef GBAUDIO_CLOCK_H
#define GBAUDIO_CLOCK_H

#include <stdint.h>

/// Generalized clock divider, supporting 2^x dividers
typedef struct gbaudio_clock_s {
    uint32_t tick;

    int divider;
} gbaudio_clock_t;

/// Initialize an audio clock divider
/// in_rate: clock rate as (2^in_rate)
/// out_rate: clock rate as (2^out_rate)
void gbaudio_clock_init(gbaudio_clock_t *clock, uint32_t in_rate, uint32_t out_rate);
/// Create an audio clock divider
gbaudio_clock_t gbaudio_clock(uint32_t in_rate, uint32_t out_rate);

/// Convenience to create the gb clock dividers.
gbaudio_clock_t sequencer_clock();
gbaudio_clock_t sweep_clock();
gbaudio_clock_t length_clock();
gbaudio_clock_t envelope_clock();

/// Step the sequencer forward by `ticks` 1Mhz clocks
/// Returns number of clocks fired at 512Hz
uint32_t gbaudio_clock_step(gbaudio_clock_t *clock, uint32_t ticks);

#endif
