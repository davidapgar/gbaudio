#define TEST_SUITE_NAME clock_tests
#include <tinyctest/tinyctest.h>

#include <gbaudio/gbaudio_clock.h>

SETUP
{
}

TEARDOWN
{
}

TEST(negative_divider)
{
    gbaudio_clock_t clock = gbaudio_clock(1, 2);
    CHECK_EQUAL(0, clock.divider, "prevent negative dividers");
}

TEST(no_tick)
{
    // 4 to 1 clock divider
    gbaudio_clock_t clock = gbaudio_clock(2, 0);
    CHECK_EQUAL(0, gbaudio_clock_step(&clock, 1), "no tick on first step");
}

TEST(single_tick)
{
    gbaudio_clock_t clock = gbaudio_clock(2, 0);
    CHECK_EQUAL(0, gbaudio_clock_step(&clock, 1), "no tick on first step");
    CHECK_EQUAL(0, gbaudio_clock_step(&clock, 1), "no tick");
    CHECK_EQUAL(0, gbaudio_clock_step(&clock, 1), "no tick");
    CHECK_EQUAL(1, gbaudio_clock_step(&clock, 1), "Tick expected after 4th");
}

TEST(double_tick)
{
    gbaudio_clock_t clock = gbaudio_clock(2, 0);
    CHECK_EQUAL(2, gbaudio_clock_step(&clock, 8), "two ticks expected");
}

TEST(tick_remain)
{
    gbaudio_clock_t clock = gbaudio_clock(2, 0);
    CHECK_EQUAL(2, gbaudio_clock_step(&clock, 10), "two ticks expected");
    CHECK_EQUAL(1, gbaudio_clock_step(&clock, 2), "one tick expected");
}

int clock_tests()
{
    RUN_TEST(no_tick);
    RUN_TEST(single_tick);
    RUN_TEST(double_tick);
    RUN_TEST(negative_divider);
    RUN_TEST(tick_remain);
    return TEST_SUITE_RESULT;
}
