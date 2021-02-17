#include <gbaudio/lfsr_gen.h>


void lfsr_gen_init(lfsr_gen_t *lfsr, int amplitude, bool width, int update_period)
{
    lfsr->amplitude = amplitude;
    lfsr->width = width;
    lfsr->update_period = update_period;

    lfsr->tick = 0;
    lfsr->reg = 1;
    lfsr->last = false;
}

int16_t lfsr_gen_next(lfsr_gen_t *lfsr, int frequency)
{
    // Handle non-initialized case
    if (lfsr->reg == 0) {
        lfsr->reg = 1;
    }

    int16_t ret;
    // Inverted
    if (lfsr->last) {
        ret = -lfsr->amplitude;
    } else {
        ret = lfsr->amplitude;
    }

    lfsr->tick += 1;
    if (lfsr->tick >= lfsr->update_period) {
        lfsr->tick -= lfsr->update_period;
    }

    if (lfsr->tick == 0) {
        uint16_t reg = lfsr->reg;

        // Get the feedback bit
        uint16_t bit0 = reg & 0x01;
        uint16_t bit1 = (reg >> 1) & 0x01;
        uint16_t feedback = bit0 ^ bit1;

        reg = reg >> 1;
        reg = reg | (feedback << 14); // Set 15th bit (bit 14)
        if (lfsr->width) {
            reg = reg | (feedback << 6); // Set bit 6
        }

        lfsr->reg = reg;
        lfsr->last = bit0;
    }

    return ret;
}
