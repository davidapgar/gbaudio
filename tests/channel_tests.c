#define TEST_SUITE_NAME channel_tests
#include <tinyctest/tinyctest.h>

#include <gbaudio/gbaudio_channel.h>


static gbaudio_channel_t channel_real;
static gbaudio_channel_t *channel;

SETUP
{
    gbaudio_channel_init(&channel_real);
    channel = &channel_real;

    //gbaudio_channel_freq(channel, 440);
    gbaudio_channel_gbfreq(channel, 1750); // ~440Hz
    gbaudio_channel_volume_envelope(channel, 0x0d, 0, 0);
    gbaudio_channel_set_amplitude(channel, 80);
    gbaudio_channel_length_duty(channel, 0, wave_duty_50);
    gbaudio_channel_sweep(channel, 1, true, 1);
    gbaudio_channel_trigger(channel, true, false);
}

TEARDOWN
{
    channel = NULL;
}

TEST(tick_moves_sequence_and_phase)
{
    gbaudio_channel_tick(channel);
    CHECK_EQUAL(1, channel->seq_clock.tick);
    CHECK_EQUAL(1, channel->phase_count);
}

TEST(duty_count)
{
    for (int i = 0; i < 298; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(1, channel->duty_count);
}

TEST(clocks)
{
    // Sequence clock is 512Hz from 1Mhz
    for (int i = 0; i < 2048; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(0, channel->seq_clock.tick);
    CHECK_EQUAL(1, channel->sweep_clock.tick);
    CHECK_EQUAL(1, channel->length_clock.tick);
    CHECK_EQUAL(1, channel->envelope_clock.tick);
}

TEST(length)
{
    gbaudio_channel_length_duty(channel, 62, wave_duty_50);
    gbaudio_channel_trigger(channel, true, true);
    // Each length tick is 4096 ticks
    for (int i = 0; i < 4096; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(1, channel->length_count);

    // Second length counter tick
    for (int i = 0; i < 4096; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(false, channel->running, "Channel should not be running");
    CHECK_EQUAL(0, gbaudio_channel_tick(channel), "Stopped channel should output silence");
}

TEST(sweep)
{
    gbaudio_channel_sweep(channel, 2, true, 1);
    gbaudio_channel_gbfreq(channel, 1024); // ~200Hz
    // Each sweep tick is 8192 ticks
    for (int i = 0; i < 8192; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(0, channel->sweep_clock.tick, "Sweep clock ticks as 128Hz");
    CHECK_EQUAL(1, channel->sweep_count, "Sweep counter ticks every 8192 APU clocks");

    // Second sweep counter tick
    for (int i = 0; i < 8192; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(0, channel->sweep_count, "sweep counter rolls over");
    uint16_t expected_freq = (1024) + (1024 >> 1);
    CHECK_EQUAL(expected_freq, channel->gbfreq, "Frequency Shift");
}

TEST(envelope)
{
    gbaudio_channel_volume_envelope(channel, 1, true, 2);
    CHECK_EQUAL(1, channel->amplitude, "Initial amplitude set from envelope");
    // Each envelope tick is 16384 ticks
    for (int i = 0; i < 16384; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(1, channel->envelope_count, "Envelope ticks every 16384 APU clocks");

    // Second envelope counter tick
    for (int i = 0; i < 16384; ++i) {
        gbaudio_channel_tick(channel);
    }
    CHECK_EQUAL(0, channel->envelope_count, "Envelope count rolls over");
    CHECK_EQUAL(2, channel->amplitude, "Amplitude increases");
}

int channel_tests()
{
    RUN_TEST(tick_moves_sequence_and_phase);
    RUN_TEST(duty_count);
    RUN_TEST(clocks);
    RUN_TEST(length);
    RUN_TEST(sweep);
    RUN_TEST(envelope);
    return TEST_SUITE_RESULT;
}
