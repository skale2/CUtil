#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "testing.h"
#include "debug.h"

/* ANSI color codes for display. */
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

test_t *running_test = NULL;

jmp_buf abort_test_loc;
jmp_buf *assert_panic_loc = NULL;

void _testing_assertion_error_(char *format, char *fsource, size_t line, ...)
{
    va_list args;
    va_start(args, line);

    char error_str[strlen(format) + strlen(fsource) + 50];

    sprintf(error_str, "\tassertion error @ %s[%zu]: %s\n", fsource, line, format);
    vprintf(error_str, args);

    va_end(args);

    testing_abort_running_test(TEST_RESULT_ASSERT_ERROR);
}

void signal_handler(int signo)
{
    switch (signo)
    {
    case SIGFPE:
        printf("\tos fault: invalid floating point operation\n");
        testing_abort_running_test(TEST_RESULT_OS_EXCEPTION);

    case SIGILL:
        printf("\tos fault: invalid instruction\n");
        testing_abort_running_test(TEST_RESULT_OS_EXCEPTION);

    case SIGINT:
        printf("\tos fault: manual interrupt\n");
        testing_abort_running_test(TEST_RESULT_OS_EXCEPTION);

    case SIGSEGV:
        printf("\tos fault: segmentation fault\n");
        testing_abort_running_test(TEST_RESULT_OS_EXCEPTION);

    case SIGTERM:
        printf("\tos fault: process terminated\n");
        testing_abort_running_test(TEST_RESULT_OS_EXCEPTION);

    case SIGBUS:
        printf("\tos fault: access of unmapped address\n");
        testing_abort_running_test(TEST_RESULT_OS_EXCEPTION);

    default:
        break;
    }
}

void register_signal_handler()
{
    if (signal(SIGFPE, signal_handler) == SIG_ERR)
        printf("testing >> warning: os floating point fault handling not supported");

    if (signal(SIGILL, signal_handler) == SIG_ERR)
        printf("testing >> warning: os invalid instruction fault handling not supported");

    if (signal(SIGINT, signal_handler) == SIG_ERR)
        printf("testing >> warning: os manual interrupt fault handling not supported");

    if (signal(SIGSEGV, signal_handler) == SIG_ERR)
        printf("testing >> warning: os segmentation fault handling not supported");

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
        printf("testing >> warning: os process terminating handling not supported");

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
        printf("testing >> warning: os unmapped address access handling not supported");
}

test_result_t testing_run(test_t *test)
{
    running_test = test;
    printf("- " KBLU "%s" KNRM "\n", test->name);

    register_signal_handler();

    before_each();

    clock_t start_time = clock();
    double ms_running;

    test_result_t result = setjmp(abort_test_loc);

    if (result)
    {
        running_test->result = result;
        ms_running = ((double)(clock() - start_time)) / CLOCKS_PER_SEC * 1000;
    }

    else
    {
        test->fn();
        ms_running = ((double)(clock() - start_time)) / CLOCKS_PER_SEC * 1000;
        running_test->result = result = TEST_RESULT_SUCCESS;
    }

    after_each();
    running_test->ms_running = ms_running;
    running_test = NULL;

    if (result == TEST_RESULT_SUCCESS)
        printf(KGRN "  pass " KNRM "(%.3lf ms)\n\n", ms_running);
    else if (result == TEST_RESULT_SKIPPED)
        printf("  skipped\n\n");
    else
        printf(KRED "  fail\n\n" KNRM);

    return result;
}

void testing_run_subsuite(test_suite_t *suite, size_t n, char *test_names[n])
{
    test_result_t result;
    printf("\n------------ Running suite: " KYEL "%s" KNRM " ------------\n\n", suite->name);

    clock_t start_time = clock();

    before_all();

    bool tests_ran[n];
    memset(tests_ran, 0, n);

    for (size_t test_idx = 0; test_idx < suite->n_tests; test_idx++)
    {
        test_t *test = &suite->tests[test_idx];
        if (test_names == NULL)
            goto test_exists;

        for (int i = 0; i < n; i++)
        {
            if (strcmp(test->name, test_names[i]) == 0)
            {
                tests_ran[i] = true;
                goto test_exists;
            }
        }
        continue;

    test_exists:
        result = testing_run(test);

        if (result == TEST_RESULT_SUCCESS)
            suite->n_successful++;
        else if (result == TEST_RESULT_SKIPPED)
            suite->n_skipped++;
        else
            suite->n_failures++;
    }

    after_all();

    double ms_running = ((double)(clock() - start_time)) / CLOCKS_PER_SEC * 1000;

    size_t suite_name_l = strlen(suite->name);
    char dash_str[suite_name_l + 1];
    memset(dash_str, '-', suite_name_l);

    printf(
        "------------ Suite results: " KYEL "%s" KNRM " ------------\n\n"
        "  " KGRN "passed" KNRM "  : %zu\n"
        "  " KRED "failed" KNRM "  : %zu\n"
        "  skipped : %zu\n"
        "  " KBLU "time" KNRM "    : %.3lf ms\n\n"
        "----------------------------%s-------------\n\n",
        suite->name, suite->n_successful, suite->n_failures, suite->n_skipped, ms_running, dash_str);

    for (int i = 0; i < n; i++)
    {
        if (!tests_ran[i])
            printf("Error: no test named '%s' in suite '%s'\n", test_names[i], suite->name);
    }
}

void testing_run_suite(test_suite_t *suite)
{
    testing_run_subsuite(suite, -1, NULL);
}

void testing_abort_running_test(test_result_t result)
{
    if (running_test == NULL)
        return;
    longjmp(abort_test_loc, result);
}

test_t *testing_get_running_test(void)
{
    return running_test;
}

void _testing_assert_(const void *item,
                      char *fsource,
                      size_t line)
{
    if (!item)
        _testing_assertion_error_("%p is not true", fsource, line, item);
}

void _testing_assert_ptr_equal_(const void *expected,
                                const void *given,
                                char *fsource,
                                size_t line)
{
    if (expected != given)
        _testing_assertion_error_("expected %p but got %p", fsource, line, expected, given);
}

void _testing_assert_ptr_not_equal_(const void *expected,
                                    const void *given,
                                    char *fsource,
                                    size_t line)
{
    if (expected == given)
        _testing_assertion_error_("didn't expected %p but got %p", fsource, line, expected, given);
}

void _testing_assert_char_equal_(char expected,
                                 char given,
                                 char *fsource,
                                 size_t line)
{
    if (expected != given)
        _testing_assertion_error_("expected '%c' but got '%c'", fsource, line, expected, given);
}

void _testing_assert_char_not_equal_(char expected,
                                     char given,
                                     char *fsource,
                                     size_t line)
{
    if (expected == given)
        _testing_assertion_error_("didn't expect '%c' but got '%c'", fsource, line, expected, given);
}

void _testing_assert_int_equal_(long long expected,
                                long long given,
                                char *fsource,
                                size_t line)
{
    if (expected != given)
        _testing_assertion_error_("expected %lld but got %lld", fsource, line, expected, given);
}

void _testing_assert_int_not_equal_(long long expected,
                                    long long given,
                                    char *fsource,
                                    size_t line)
{
    if (expected == given)
        _testing_assertion_error_("didn't expected %lld but got %lld", fsource, line, expected, given);
}

void _testing_assert_uint_equal_(unsigned long long expected,
                                 unsigned long long given,
                                 char *fsource,
                                 size_t line)
{
    if (expected != given)
        _testing_assertion_error_("expected %zu but got %zu", fsource, line, expected, given);
}

void _testing_assert_uint_not_equal_(unsigned long long expected,
                                     unsigned long long given,
                                     char *fsource,
                                     size_t line)
{
    if (expected == given)
        _testing_assertion_error_("%zu but got %zu", fsource, line, expected, given);
}

void _testing_assert_double_equal_(long double expected,
                                   long double given,
                                   char *fsource,
                                   size_t line)
{
    _testing_assert_double_equal_prec_(expected, given, testing_DEFAULT_FP_ROUNDING_EPS, fsource, line);
}

void _testing_assert_double_not_equal_(long double expected,
                                       long double given,
                                       char *fsource,
                                       size_t line)
{
    _testing_assert_double_not_equal_prec_(expected, given, testing_DEFAULT_FP_ROUNDING_EPS, fsource, line);
}

void _testing_assert_double_equal_prec_(long double expected,
                                        long double given,
                                        double prec,
                                        char *fsource,
                                        size_t line)
{
    if (fabsl(expected - given) > pow(10, prec))
        _testing_assertion_error_("expected to be within %d of %d but got %d", fsource, line, prec, expected, given);
}

void _testing_assert_double_not_equal_prec_(long double expected,
                                            long double given,
                                            double prec,
                                            char *fsource,
                                            size_t line)
{
    if (fabsl(expected - given) <= pow(10, prec))
        _testing_assertion_error_("didn't expect to be within %d of %d but got %d", fsource, line, prec, expected, given);
}

void _testing_assert_null_(const void *item,
                           char *fsource,
                           size_t line)
{
    if (item != NULL)
        _testing_assertion_error_("%p is not NULL", fsource, line, item);
}

void _testing_assert_not_null_(const void *item,
                               char *fsource,
                               size_t line)
{
    if (item == NULL)
        _testing_assertion_error_("%p is NULL", fsource, line, item);
}

void _testing_assert_strings_equal_(const char *expected,
                                    const char *given,
                                    char *fsource,
                                    size_t line)
{
    if (strcmp(expected, given) != 0)
        _testing_assertion_error_("expected %s but got %s", fsource, line, expected, given);
}

void _testing_assert_strings_not_equal_(const char *expected,
                                        const char *given,
                                        char *fsource,
                                        size_t line)
{
    if (strcmp(expected, given) == 0)
        assertion_error("didn't expect \"%s\" but got \"%s\"", fsource, line, expected, given);
}

void _testing_assert_mem_equal_(const void *expected,
                                const void *given,
                                size_t size,
                                char *fsource,
                                size_t line)
{
    uint8_t *fref = (uint8_t *)expected;
    uint8_t *sref = (uint8_t *)given;

    for (size_t i = 0; i < size; i++, fref++, sref++)
    {
        if (*fref != *sref)
            _testing_assertion_error_(
                "expected %p but got %p at position %zu in buffers %p and %p respectively",
                fsource, line, fref, sref, i, expected, given);
    }
}

void _testing_assert_mem_not_equal_(const void *expected,
                                    const void *given,
                                    size_t size,
                                    char *fsource,
                                    size_t line)
{
    uint8_t *fref = (uint8_t *)expected;
    uint8_t *sref = (uint8_t *)given;

    for (size_t i = 0; i < size; i++, fref++, sref++)
    {
        if (*fref != *sref)
            return;
    }

    _testing_assertion_error_("didn't expected %p and %p to be equal for %zu bytes",
                              fsource, line, fref, sref, size);
}

void _testing_assert_arrays_equal_(const void *expected,
                                   const void *given,
                                   size_t nitems,
                                   char *type_name,
                                   size_t type_size,
                                   char *fsource,
                                   size_t line)
{
    uint8_t *fref = (uint8_t *)expected;
    uint8_t *sref = (uint8_t *)given;

    for (size_t i = 0; i < nitems; i++, fref += type_size, sref += type_size)
    {
        if (memcmp(fref, sref, type_size) == 0)
            _testing_assertion_error_(
                "expected %zu of %s arrays %p and %p to be equal",
                fsource, line, fref, sref, i, type_name, expected, given);
    }
}

void _testing_assert_arrays_not_equal_(const void *expected,
                                       const void *given,
                                       size_t nitems,
                                       char *type_name,
                                       size_t type_size,
                                       char *fsource,
                                       size_t line)
{
    uint8_t *fref = (uint8_t *)expected;
    uint8_t *sref = (uint8_t *)given;

    for (size_t i = 0; i < nitems; i++, fref += type_size, sref += type_size)
    {
        if (memcmp(fref, sref, type_size) != 0)
            return;
    }

    _testing_assertion_error_(
        "didn't expect %s arrays %p and %p to be equal equal",
        fsource, line, type_name, expected, given);
}

void _testing_assert_true_(bool pred,
                           char *fsource,
                           size_t line)
{
    if (!pred)
        _testing_assertion_error_("expected true but got false", fsource, line);
}

void _testing_assert_false_(bool pred,
                            char *fsource,
                            size_t line)
{
    if (pred)
        _testing_assertion_error_("expected false but got true", fsource, line);
}

void _testing_fail_(char *fsource,
                    size_t line)
{
    printf("\tFailure @ %s[%zu]\n", fsource, line);
    testing_abort_running_test(TEST_RESULT_FAILURE);
}

jmp_buf *testing_panic_handler_register(jmp_buf *new_loc)
{
    jmp_buf *old_loc = assert_panic_loc;
    assert_panic_loc = new_loc;
    return old_loc;
}

bool testing_panic_handler_exists()
{
    return assert_panic_loc != NULL;
}

void testing_panic_handler_catch()
{
    if (!testing_panic_handler_exists())
        return;
    longjmp(*assert_panic_loc, 1);
}

void testing_panic_handler_reset(jmp_buf *prev_loc)
{
    assert_panic_loc = prev_loc;
}

void testing_seed_rand(unsigned int seed)
{
    srand(seed);
}

int testing_rand_int()
{
    return testing_rand_int_range(INT32_MIN, INT32_MAX);
}

int testing_rand_int_range(int min, int max)
{
    return (int)(testing_rand_double() * (max - min) + min);
}

double testing_rand_double()
{
    return ((double)rand()) / RAND_MAX;
}

double testing_rand_double_range(double min, double max)
{
    return testing_rand_double() * (max - min) + min;
}

void testing_rand_mem(void *buffer, size_t size)
{
    for (uint32_t *i = buffer; i < (uint32_t *)buffer + size; i++)
        *i = testing_rand_int();
}