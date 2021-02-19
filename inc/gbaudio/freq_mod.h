#ifndef FREQ_MOD_H
#define FREQ_MOD_H

#include <gbaudio/audio_gen.h>
#include <gbaudio/delta_gen.h>

/// Frequency modulate with two underlying generators.
/// The carrier signal must support adjust_frequency.
typedef struct freq_mod_s {
    audio_gen_t *carrier;
    delta_gen_t modulator;
    // Modulator as an audio generator.
    audio_gen_t mod_a;
} freq_mod_t;

void freq_mod_init(freq_mod_t *freq_mod, audio_gen_t *carrier, audio_gen_t *modulator);
int16_t freq_mod_next(freq_mod_t *freq_mod, int frequency);
audio_gen_t freq_mod_to_audio_gen(freq_mod_t *freq_mod);

#endif
