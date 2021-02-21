#ifndef TINYCTEST_H
#define TINYCTEST_H

#ifndef SKIP_INCLUDE
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#endif

#ifndef TEST_SUITE_NAME
#define TEST_SUITE_NAME default
#endif

#define TEST_STATE_BUF_SIZE 128

#define _STRINGIFY(str) #str
#define STRINGIFY(str) _STRINGIFY(str)
#define TEST_SUITE_STRING STRINGIFY(TEST_SUITE_NAME)

#define _TEST_SUITE_PREFIX(prefix, name) TEST_ ## prefix ## _ ## name
#define TEST_SUITE_PREFIX(prefix, name) _TEST_SUITE_PREFIX(prefix, name)

#define TEST_SUITE_RESULT TEST_SUITE_PREFIX(TEST_SUITE_NAME, test_result)
static int TEST_SUITE_RESULT = 0;

typedef struct test_state_s
{
    int TEST_STATE_pass;
    int TEST_STATE_line;
    char *TEST_STATE_name;
    char *TEST_STATE_condition;
    char TEST_STATE_failure_message[TEST_STATE_BUF_SIZE];
    char TEST_STATE_message[TEST_STATE_BUF_SIZE];
} test_state_t;

static test_state_t global_test_state;

static void TEST_STATE_reset(test_state_t *test_state)
{
    test_state->TEST_STATE_pass = 1;
    test_state->TEST_STATE_line = 0;
    test_state->TEST_STATE_name = "unknown";
    test_state->TEST_STATE_condition = "undefined";
    test_state->TEST_STATE_failure_message[0] = '\0';
    test_state->TEST_STATE_message[0] = '\0';
}

static int TEST_STATE_evaluate(test_state_t *test_state, int line, char *condition, int result, char *format, ...)
{
    if (!result) {
        test_state->TEST_STATE_pass = 0;
        test_state->TEST_STATE_line = line;
        test_state->TEST_STATE_condition = condition;
        if (format[0] != '\0') {
            va_list valist;
            va_start(valist, format);
            vsnprintf(test_state->TEST_STATE_message, TEST_STATE_BUF_SIZE, format, valist);
            test_state->TEST_STATE_message[TEST_STATE_BUF_SIZE-1] = '\0';
            va_end(valist);
        }
        TEST_SUITE_RESULT = 1;
    }

    return result;
}

static void TEST_STATE_print_group(char *group_name)
{
    fprintf(stderr, "\n=== TEST GROUP: %s ===\n", group_name);
}

static void TEST_STATE_print_result(test_state_t *test_state)
{
    static int has_printed_group = 0;
    if (!has_printed_group) {
        TEST_STATE_print_group(TEST_SUITE_STRING);
        has_printed_group = 1;
    }

    fprintf(stderr, "TEST %s: ", test_state->TEST_STATE_name);
    if (test_state->TEST_STATE_pass) {
        fprintf(stderr, "Passed\n");
    } else {
        fprintf(stderr, "Failed %s at line %d",
                test_state->TEST_STATE_condition,
                test_state->TEST_STATE_line);
        fprintf(stderr, ": %s", test_state->TEST_STATE_message);
        if (test_state->TEST_STATE_failure_message[0]) {
            fprintf(stderr, " (%s)", test_state->TEST_STATE_failure_message);
        }
        fprintf(stderr, "\n");
    }
}

#define SETUP static void TEST_SUITE_PREFIX(TEST_SUITE_NAME, setUp)()
#define TEARDOWN static void TEST_SUITE_PREFIX(TEST_SUITE_NAME, tearDown)()

// Check types

#define _CHECK_PRIMITIVE(test_state, condition, result, msg...) \
do { \
    if (!TEST_STATE_evaluate(test_state, __LINE__, #condition, result, "" msg)) { \
        return; \
    } \
} while (0)

#define CHECK(condition, msg...) _CHECK_PRIMITIVE(test_state, condition, (condition), msg)

#define CHECK_EQUAL(expected, actual, msg...) \
do { \
    long CHECK_expected = (long)(expected); \
    long CHECK_actual = (long)(actual); \
    int CHECK_result = (CHECK_expected == CHECK_actual); \
    snprintf(test_state->TEST_STATE_failure_message, TEST_STATE_BUF_SIZE, "%ld (0x%lx) is not equal to %ld (0x%lx)", CHECK_actual, CHECK_actual, CHECK_expected, CHECK_expected); \
    _CHECK_PRIMITIVE(test_state, expected == actual, CHECK_result, msg); \
} while (0)

#define CHECK_EQUAL_PTR(expected, actual, msg...) \
do { \
    void const *CHECK_expected = (expected); \
    void const *CHECK_actual = (actual); \
    int CHECK_result = (CHECK_expected == CHECK_actual); \
    snprintf(test_state->TEST_STATE_failure_message, TEST_STATE_BUF_SIZE, "%p is not equal to %p", CHECK_actual, CHECK_expected); \
    _CHECK_PRIMITIVE(test_state, expected == actual, CHECK_result, msg); \
} while (0)

#define CHECK_EQUAL_STR(expected, actual, msg...) \
do { \
    char const* CHECK_expected = (expected); \
    char const* CHECK_actual = (actual); \
    int CHECK_result = (strcmp(CHECK_expected, CHECK_actual) == 0); \
    snprintf(test_state->TEST_STATE_failure_message, TEST_STATE_BUF_SIZE, "\"%s\" is not equal to \"%s\"", CHECK_actual, CHECK_expected); \
    _CHECK_PRIMITIVE(test_state, expected == actual, CHECK_result, msg); \
} while (0)

#define CHECK_ZERO(actual, msg...) \
do { \
    CHECK_EQUAL(0, actual, msg); \
} while (0)

// Test definitions and running

#define TESTNAME(name) TEST_SUITE_PREFIX(TEST_SUITE_NAME, name)
#define TEST(testname) static void TESTNAME(testname)(test_state_t *test_state)

#define _RUN_TEST_PRIMITIVE(testname, test_state, should_print) \
do { \
    TEST_STATE_reset(test_state); \
    TEST_SUITE_PREFIX(TEST_SUITE_NAME, setUp)(); \
    (test_state)->TEST_STATE_name = #testname; \
    TESTNAME(testname)(test_state); \
    if (should_print) \
        TEST_STATE_print_result(test_state); \
    TEST_SUITE_PREFIX(TEST_SUITE_NAME, tearDown)(); \
} while (0)

#define RUN_TEST(testname) \
do { \
    _RUN_TEST_PRIMITIVE(testname, &global_test_state, true); \
} while (0)

#endif
