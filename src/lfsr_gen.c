#include <gbaudio/lfsr_gen.h>

#include <stddef.h>


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

void lfsr_gen_adjust_amplitude(lfsr_gen_t *lfsr, int amp)
{
    int amplitude = lfsr->amplitude + amp;
    if (amplitude > 32768) {
        amplitude = 32768;
    } else if (amplitude < 0) {
        amplitude = 0;
    }
    lfsr->amplitude = amplitude;
}

void lfsr_gen_adjust_period(lfsr_gen_t *lfsr, int per)
{
    int period = lfsr->update_period + per;
    if (period > 32) {
        period = 32;
    } else if (period < 0) {
        period = 0;
    }
    lfsr->update_period = period;
}

void lfsr_gen_cycle_width(lfsr_gen_t *lfsr)
{
    bool width = !lfsr->width;
    lfsr->width = width;
}

static int16_t audio_lfsr_gen_next(void *generator, int freq)
{
    lfsr_gen_t *lfsr = (lfsr_gen_t *)generator;
    return lfsr_gen_next(lfsr, freq);
}

static void audio_lfsr_adjust_amplitude(void *generator, int amp)
{
    lfsr_gen_t *lfsr_gen = (lfsr_gen_t *)generator;
    lfsr_gen_adjust_amplitude(lfsr_gen, amp);
}

static int audio_lfsr_get_amplitude(void *generator)
{
    lfsr_gen_t *lfsr_gen = (lfsr_gen_t *)generator;
    return lfsr_gen->amplitude;
}

audio_gen_t lfsr_to_audio_gen(lfsr_gen_t *lfsr)
{
    audio_gen_t ret = {
        .generator = lfsr,
        .next = audio_lfsr_gen_next,
        .adjust_amplitude = audio_lfsr_adjust_amplitude,
        .get_amplitude = audio_lfsr_get_amplitude,
        .adjust_frequency = NULL,
        .get_frequency = NULL,
    };
    return ret;
}
