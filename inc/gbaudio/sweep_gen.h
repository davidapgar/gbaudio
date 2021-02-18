#ifndef SWEEP_GEN_H
#define SWEEP_GEN_H

#include <stdbool.h>
#include <stdint.h>

#include <gbaudio/audio_gen.h>


/// Frequency sweep for an audio generator.
typedef struct sweep_gen_s {
    audio_gen_t *audio_gen;
    /// Passthrough to underlying generator (maybe)
    int amplitude;
    /// Positive or negative frequency change
    bool change;
    /// Frequency of updates (default 128Hz)
    int frequency;
    /// How many periods per update, 0-7
    int time;
    /// How many sweeps, 0-7
    int n_sweep;
    /// Shift of the sweep ("n" in X(t) = X(t-1) +/- X(t-1)/2^n), 0-7
    int shift;

    int tick;
} sweep_gen_t;

void sweep_gen_init(sweep_gen_t *sweep_gen, audio_gen_t *audio_gen, bool change, int time, int n_sweep, int shift);
/// Reset tick and start sweeping again.
void sweep_gen_reset(sweep_gen_t *sweep_gen);

audio_gen_t sweep_to_audio_gen(sweep_gen_t *sweep_gen);

#endif
