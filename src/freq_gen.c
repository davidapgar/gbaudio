#include <gbaudio/freq_gen.h>


void freq_gen_init(freq_gen_t *gen, int amplitude, int frequency, duty_cycle_t duty)
{
    // TODO: Clamp these to reasonable values?
    gen->amplitude = amplitude;
    gen->note_hz = frequency;
    gen->duty = duty;
    gen->ticks = 0;
}

int16_t freq_gen_next(freq_gen_t *gen, int sample_freq)
{
    int period = sample_freq / gen->note_hz;
    int ticks = gen->ticks;
    if (ticks >= period) {
        ticks -= period;
    }
    int16_t ret;

    int duty = period / 8;
    switch (gen->duty) {
    case 0:
        // 12.5% down duty cycle
        break;
    case 1:
        // 25%
        duty = duty * 2;
        break;
    case 2:
        // 50%
        duty = duty * 4;
        break;
    case 3:
        // 75%
        duty = duty * 6;
        break;
    }
    if (ticks < duty) {
        ret = gen->amplitude * -1;
    } else {
        ret = gen->amplitude;
    }
    gen->ticks = (ticks + 1);
    return ret;
}
