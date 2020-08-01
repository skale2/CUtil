#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "functions.h"

#define testing_ENABLE_ALIASES true

#define testing_DEFAULT_FP_ROUNDING_EPS 1e-5

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/* ------------------------------------------------------------- *
 *            ---------- resource handlers ----------            *
 * ------------------------------------------------------------- */

void before_each(void);
void after_each(void);

void before_all(void);
void after_all(void);

#define DEFAULT_RESOURCE_HANDLERS \
    void before_each(void) {}     \
    void after_each(void) {}      \
    void before_all(void) {}      \
    void after_all(void) {}    

#define DEFAULT_RESOURCE_HANDLER_EACH \
    void before_each(void) {}         \
    void after_each(void) {}

#define DEFAULT_RESOURCE_HANDLER_ALL \
    void before_all(void) {}         \
    void after_all(void) {}

/* ------------------------------------------------------------- *
 *                 ---------- aliases ----------                 *
 * ------------------------------------------------------------- */

#if testing_ENABLE_ALIASES
#define assert(item)                                             (testing_assert((item)))
#define assert_equal(expected, given)                            (testing_assert_equal((expected), (given)))
#define assert_not_equal(expected, given)                        (testing_assert_not_equal((expected), (given)))
#define assert_int_equal(expected, given)                        (testing_assert_int_equal((expected), (given)))
#define assert_int_not_equal(expected, given)                    (testing_assert_int_not_equal((expected), (given)))
#define assert_uint_equal(expected, given)                       (testing_assert_uint_equal((expected), (given)))
#define assert_uint_not_equal(expected, given)                   (testing_assert_uint_not_equal((expected), (given)))
#define assert_size_equal(expected, given)                       (testing_assert_size_equal((expected), (given)))
#define assert_size_not_equal(expected, given)                   (testing_assert_size_not_equal((expected), (given)))
#define assert_double_equal(expected, given, prec)               (testing_assert_double_equal((expected), (given), (prec)))
#define assert_double_not_equal(expected, given, prec)           (testing_assert_double_not_equal((expected), (given), (prec)))
#define assert_null(item)                                        (testing_assert_null((item))))
#define assert_not null(item)                                    (testing_assert_not_null((item)))
#define assert_strings_equal(expected, given)                    (testing_assert_strings_equal((expected), (given)))
#define assert_strings_not_equal(expected, given)                (testing_assert_strings_not_equal((expected), (given)))
#define assert_mem_equal(size, expected, given)                  (testing_assert_mem_equal((size), (expected), (expected)))
#define assert_mem_not_equal(size, expected, given)              (testing_assert_mem_not_equal((size), (expected), (given)))
#define assert_struct_equal(expected, given)                     (testing_assert_struct_equal((expected), (given)))
#define assert_struct_not_equal(expected, given)                 (testing_assert_struct_not_equal((expected), (given)))
#define assert_arrays_equal(item_t, nitems, expected, given)     (testing_assert_arrays_equal(item_t, (nitems), (expected), (given)))
#define assert_arrays_not_equal(item_t, nitems, expected, given) (testing_assert_arrays_not_equal(item_t, (nitems), (expected), (given)))
#define assert_true(pred)                                        (testing_assert_true((pred)))
#define assert_false(pred)                                       (testing_assert_false((pred)))
#define assert_panic(expr)                                       (testing_assert_panic((expr)))
#define fail()                                                   (testing_fail())

#define seed_rand(seed)                                          (testing_seed_rand((seed)))
#define rand_int()                                               (testing_rand_int())
#define rand_int_range(min, max)                                 (testing_rand_int_range(min, (max)))
#define rand_double()                                            (testing_rand_double())
#define rand_double_range(min, max)                              (testing_rand_double_range((min), (max)))
#define rand_mem(buffer, size)                                   (testing_rand_mem((buffer), (size)))
#endif // testing_ENABLE_ALIASES

/* ------------------------------------------------------------- *
 *                  ---------- tests ----------                  *
 * ------------------------------------------------------------- */

typedef enum test_result
{
    TEST_RESULT_SUCCESS = 1,
    TEST_RESULT_PENDING = 2,
    TEST_RESULT_NONTERM = 3,
    TEST_RESULT_PANIC   = 4,
    TEST_RESULT_SKIPPED = 5,
    TEST_RESULT_FAILURE = 6,
    TEST_RESULT_ASSERT_ERROR = 7,
    TEST_RESULT_OS_EXCEPTION = 8
} test_result_t;

typedef void (*test_fn_t)(void);

typedef struct test
{
    char *name;
    test_fn_t fn;
    struct test *next;
    double ms_running;
    test_result_t result;
} test_t;

typedef struct test_suite
{
    char *name;
    size_t n_tests;
    size_t n_successful, n_failures, n_skipped;
    test_t *tests;
} test_suite_t;

#define TEST(test_fn) \
    ((test_t){ .name = #test_fn, .fn = (test_fn), .result = TEST_RESULT_PENDING })

#define TEST_SUITE(...)                                \
    ({                                                 \
        test_t tests[] = {__VA_ARGS__};                \
        test_suite_t test_suite =                      \
        {                                              \
            .name = __FILENAME__,                      \
            .n_tests = sizeof(tests) / sizeof(test_t), \
            .n_successful = 0,                         \
            .n_failures = 0,                           \
            .n_skipped = 0,                            \
            .tests = tests                             \
        };                                             \
        testing_run_subsuite(                          \
            &test_suite,                               \
            argc - 1,                                  \
            argc >= 2 ? &argv[1] : NULL                \
        );                                             \
    })

#define SKIP_TEST \
    (testing_abort_running_test(TEST_RESULT_SKIPPED));

test_result_t  testing_run                (test_t *);
void           testing_run_suite          (test_suite_t *);
void           testing_run_subsuite       (test_suite_t *, size_t n, char *test_names[n]);
void           testing_abort_running_test (test_result_t);
test_t        *testing_get_running_test   (void);

/* ------------------------------------------------------------- *
 *               ---------- assertions ----------                *
 * ------------------------------------------------------------- */

#define testing_assert_equal(_expected_, _given_)                                                 \
    ({                                                                                            \
        typeof(_expected_) expected = (_expected_);                                               \
        typeof(_expected_) given = (_given_);                                                     \
                                                                                                  \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), char),               \
              _testing_assert_char_equal_,                                                        \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), short) ||            \
                              __builtin_types_compatible_p(typeof(expected), int) ||              \
                              __builtin_types_compatible_p(typeof(expected), long) ||             \
                              __builtin_types_compatible_p(typeof(expected), long long),          \
            _testing_assert_int_equal_,                                                           \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), unsigned short) ||   \
                              __builtin_types_compatible_p(typeof(expected), unsigned int) ||     \
                              __builtin_types_compatible_p(typeof(expected), unsigned long) ||    \
                              __builtin_types_compatible_p(typeof(expected), unsigned long long), \
            _testing_assert_uint_equal_,                                                          \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), size_t),             \
            _testing_assert_uint_equal_,                                                          \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), float) ||            \
                              __builtin_types_compatible_p(typeof(expected), double),             \
            _testing_assert_double_equal_,                                                        \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), void *),             \
            _testing_assert_ptr_equal_,                                                           \
        __builtin_choose_expr(__builtin_types_compatible_p(typeof(expected), char *),             \
            _testing_assert_strings_equal_,                                                       \
            NULL)))))))                                                                           \
                (expected, given, __FILENAME__, __LINE__);                                        \
    })

#define testing_assert(item)                                             (_testing_assert_((item), __FILENAME__, __LINE__))
#define testing_assert_ptr_equal(expected, given)                        (_testing_assert_ptr_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_ptr_not_equal(expected, given)                    (_testing_assert_ptr_not_equal_((expected), given, __FILENAME__, __LINE__))
#define testing_assert_char_equal(expected, given)                       (_testing_assert_char_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_char_not_equal(expected, given)                   (_testing_assert_char_not_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_char_not_equal(expected, given)                   (_testing_assert_char_not_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_int_equal(expected, given)                        (_testing_assert_int_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_int_not_equal(expected, given)                    (_testing_assert_int_not_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_uint_equal(expected, given)                       (_testing_assert_uint_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_uint_not_equal(expected, given)                   (_testing_assert_uint_not_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_size_equal(expected, given)                       (_testing_assert_uint_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_size_not_equal(expected, given)                   (_testing_assert_uint_not_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_double_equal(expected, given)                     (_testing_assert_double_equal((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_double_not_equal(expected, given)                 (_testing_assert_double_not_equal((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_double_equal_prec(expected, given, prec)          (_testing_assert_double_equal_prec_((expected), (given), (prec), __FILENAME__, __LINE__))
#define testing_assert_double_not_equal_prec(expected, given, prec)      (_testing_assert_double_not_equal_prec_((expected), (given), (prec), __FILENAME__, __LINE__))
#define testing_assert_null(item)                                        (_testing_assert_null_((item), __FILENAME__, __LINE__))
#define testing_assert_not null(item)                                    (_testing_assert_not_null_((item), __FILENAME__, __LINE__))
#define testing_assert_strings_equal(expected, given)                    (_testing_assert_strings_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_strings_not_equal(expected, given)                (_testing_assert_strings_not_equal_((expected), (given), __FILENAME__, __LINE__))
#define testing_assert_mem_equal(size, expected, given)                  (_testing_assert_mem_equal_((expected), (given), (size), __FILENAME__, __LINE__))
#define testing_assert_mem_not_equal(size, expected, given)              (_testing_assert_mem_not_equal_((expected), (given), (size), __FILENAME__, __LINE__))
#define testing_assert_arrays_equal(item_t, nitems, expected, given)     (_testing_assert_arrays_equal_((expected), (given), (nitems), #item_t, sizeof((item_t)), __FILENAME__, __LINE__))
#define testing_assert_arrays_not_equal(item_t, nitems, expected, given) (_testing_assert_arrays_not_equal_((expected), (given), (nitems), #item_t, sizeof((item_t)), __FILENAME__, __LINE__))
#define testing_assert_true(pred)                                        (_testing_assert_true_((pred), __FILENAME__, __LINE__))
#define testing_assert_false(pred)                                       (_testing_assert_false_((pred), __FILENAME__, __LINE__))
#define assertion_error(format, ...)                                     (_testing_assertion_error_((format), __FILENAME__, __LINE__, ## __VA_ARGS__))
#define testing_fail()                                                   (_testing_fail_(__FILENAME__, __LINE__))

#define testing_assert_struct_equal(expected, given) \
    ({                                               \
        typeof(expected) f = expected;               \
        typeof(expected) s = given;                  \
        _testing_assert_mem_equal_(                  \
            &f, &s,                                  \
            sizeof(typeof(expected)),                \
            __FILENAME__,                            \
            __LINE__);                               \
    })
#define testing_assert_struct_not_equal(expected, given) \
    ({                                                   \
        typeof(expected) f = expected;                   \
        typeof(expected) s = given;                      \
        _testing_assert_mem_not_equal_(                  \
            &f, &s,                                      \
            sizeof(typeof(expected)),                    \
            __FILENAME__,                                \
            __LINE__);                                   \
    })

#define testing_assert_panic(expr)                                \
    ({                                                            \
        jmp_buf buf;                                              \
        jmp_buf *prev_buf = testing_panic_handler_register(&buf); \
        bool did_panic = false;                                   \
        if (setjmp(buf))                                          \
            did_panic = true;                                     \
        else                                                      \
            /* Cast to void to avoid unused value warning */      \
            (void)(expr);                                         \
        testing_panic_handler_reset(prev_buf);                    \
        if (!did_panic)                                           \
            assertion_error("did not panic");                     \
    })

void _testing_assert_ptr_equal_              (const void *, const void *, char *, size_t);
void _testing_assert_ptr_not_equal_          (const void *, const void *, char *, size_t);
void _testing_assert_char_equal_             (char, char, char *, size_t);
void _testing_assert_char_not_equal_         (char, char, char *, size_t);
void _testing_assert_int_equal_              (long long, long long, char *, size_t);
void _testing_assert_int_not_equal_          (long long, long long, char *, size_t);
void _testing_assert_uint_equal_             (unsigned long long, unsigned long long, char *, size_t);
void _testing_assert_uint_not_equal_         (unsigned long long, unsigned long long, char *, size_t);
void _testing_assert_double_equal_           (long double, long double, char *, size_t);
void _testing_assert_double_not_equal_       (long double, long double, char *, size_t);
void _testing_assert_double_equal_prec_      (long double, long double, double eps, char *, size_t);
void _testing_assert_double_not_equal_prec_  (long double, long double, double eps, char *, size_t);
void _testing_assert_null_                   (const void *, char *, size_t);
void _testing_assert_not_null_               (const void *, char *, size_t);
void _testing_assert_strings_equal_          (const char *, const char *, char *, size_t);
void _testing_assert_strings_not_equal_      (const char *, const char *, char *, size_t);
void _testing_assert_mem_equal_              (const void *, const void *, size_t, char *, size_t);
void _testing_assert_mem_not_equal_          (const void *, const void *, size_t, char *, size_t);
void _testing_assert_arrays_equal_           (const void *, const void *, size_t, char *, size_t, char *, size_t);
void _testing_assert_arrays_not_equal_       (const void *, const void *, size_t, char *, size_t, char *, size_t);
void _testing_assert_true_                   (bool, char *, size_t);
void _testing_assert_false_                  (bool, char *, size_t);
bool _testing_check_panic_did_catch_         ();
void _testing_panic_did_catch_               ();
void _testing_assertion_error_               (char *format, char *fsource, size_t line, ...);
void _testing_fail_                          (char *, size_t);
     
jmp_buf *testing_panic_handler_register      (jmp_buf *new_loc);
bool     testing_panic_handler_exists        ();
void     testing_panic_handler_catch         ();
void     testing_panic_handler_reset         (jmp_buf *prev_loc);

/* ------------------------------------------------------------- *
 *               ---------- randomness ----------                *
 * ------------------------------------------------------------- */

void   testing_seed_rand         (unsigned int seed);
int    testing_rand_int          ();
int    testing_rand_int_range    (int, int);
double testing_rand_double       ();
double testing_rand_double_range (double, double);
void   testing_rand_mem          (void *, size_t);