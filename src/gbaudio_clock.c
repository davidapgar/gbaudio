#include <gbaudio/gbaudio_clock.h>


void gbaudio_clock_init(gbaudio_clock_t *clock, uint32_t in_rate, uint32_t out_rate)
{
    uint32_t divider;
    if (in_rate < out_rate) {
        divider = 0;
    } else {
        divider = in_rate - out_rate;
    }

    clock->tick = 0;
    clock->divider = divider;
}

gbaudio_clock_t gbaudio_clock(uint32_t in_rate, uint32_t out_rate)
{
    gbaudio_clock_t ret;
    gbaudio_clock_init(&ret, in_rate, out_rate);
    return ret;
}

gbaudio_clock_t sequencer_clock()
{
    // Input of 1Mhz (2^20)
    // Output of 512Hz (2^9)
    return gbaudio_clock(20, 9);
}

gbaudio_clock_t sweep_clock()
{
    // Input of 512Hz (2^9)
    // Output of 128Hz (2^7)
    return gbaudio_clock(9, 7);
}

gbaudio_clock_t length_clock()
{
    // Input of 512Hz (2^9)
    // Output of 256 (2^8)
    return gbaudio_clock(9, 8);
}

gbaudio_clock_t envelope_clock()
{
    // Input of 512Hz (2^9)
    // Output of 64 (2^6)
    return gbaudio_clock(9, 6);
}

uint32_t gbaudio_clock_step(gbaudio_clock_t *clock, uint32_t ticks)
{
    ticks += clock->tick;

    uint32_t mask = ~((~(uint32_t)0) << clock->divider);

    uint32_t fired = ticks >> clock->divider;
    uint32_t remainder = ticks & mask;
    clock->tick = remainder;

    return fired;
}
