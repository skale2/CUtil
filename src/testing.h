#ifndef TESTING_H
#define TESTING_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#define testing__ENABLE_ALIASES true

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

#if testing__ENABLE_ALIASES
#define assert(item)                                           (testing_assert((item)))
#define assert_equal(first, second)                            (testing_assert_equal((first), (second)))
#define assert_not_equal(first, second)                        (testing_assert_not_equal((first), (second)))
#define assert_int_equal(first, second)                        (testing_assert_int_equal((first), (second)))
#define assert_int_not_equal(first, second)                    (testing_assert_int_not_equal((first), (second)))
#define assert_uint_equal(first, second)                       (testing_assert_uint_equal((first), (second)))
#define assert_uint_not_equal(first, second)                   (testing_assert_uint_not_equal((first), (second)))
#define assert_double_equal(first, second, prec)               (testing_assert_double_equal((first), (second), (prec)))
#define assert_double_not_equal(first, second, prec)           (testing_assert_double_not_equal((first), (second), (prec)))
#define assert_null(item)                                      (testing_assert_null((item))))
#define assert_not null(item)                                  (testing_assert_not_null((item)))
#define assert_strings_equal(first, second)                    (testing_assert_strings_equal((first), (second)))
#define assert_strings_not_equal(first, second)                (testing_assert_strings_not_equal((first), (second)))
#define assert_mem_equal(first, second, size)                  (testing_assert_mem_equal((first), (first), (size)))
#define assert_mem_not_equal(first, second, size)              (testing_assert_mem_not_equal((first), (second), (size)))
#define assert_structs_equal(first, second, type)              (testing_assert_structs_equal((first), (second), (type)))
#define assert_structs_not_equal(first, second, type)          (testing_assert_structs_not_equal((first), (second), (type)))
#define assert_arrays_equal(first, second, nitems, item_t)     (testing_assert_arrays_equal((first), (second), (nitems), item_t))
#define assert_arrays_not_equal(first, second, nitems, item_t) (testing_assert_arrays_not_equal((first), (second), (nitems), item_t))
#define assert_true(pred)                                      (testing_assert_true((pred)))
#define assert_false(pred)                                     (testing_assert_false((pred)))
#define assert_panic(expr)                                     (testing_assert_panic((expr)))
#define fail()                                                 (testing_fail())
           
#define seed_rand(seed)                                        (testing_seed_rand((seed)))
#define rand_int()                                             (testing_rand_int())
#define rand_int_range(min, max)                               (testing_rand_int_range(min, (max)))
#define rand_double()                                          (testing_rand_double())
#define rand_double_range(min, max)                            (testing_rand_double_range((min), (max)))
#define rand_mem(buffer, size)                                 (testing_rand_mem((buffer), (size)))
#endif

/* ------------------------------------------------------------- *
 *                  ---------- tests ----------                  *
 * ------------------------------------------------------------- */

typedef enum test_result
{
	TEST_RESULT_SUCCESS = 1,
	TEST_RESULT_PENDING = 2,
	TEST_RESULT_NONTERM = 3,
	TEST_RESULT_PANIC   = 4,
	TEST_RESULT_FAILURE = 5,
	TEST_RESULT_ASSERT_ERROR = 6,
	TEST_RESULT_OS_EXCEPTION = 7
} test_result_t;

typedef void (*test_fn_t)(void);

typedef struct test
{
	char *name;
	test_fn_t fn;
	struct test *next;
	test_result_t result;
} test_t;

typedef struct test_suite
{
	char *name;
	size_t n_tests;
	size_t n_successful, n_failures;
	test_t *tests;
} test_suite_t;

#define TEST(test_fn) ((test_t){ .name = #test_fn, .fn = (test_fn), .result = TEST_RESULT_PENDING })

#define TEST_SUITE(...)                                \
	({                                                 \
		test_t tests[] = {__VA_ARGS__};                \
		test_suite_t test_suite =                      \
		{                                              \
			.name = __FILENAME__,                      \
			.n_tests = sizeof(tests) / sizeof(test_t), \
			.n_successful = 0,                         \
			.n_failures = 0,                           \
			.tests = tests                             \
		};                                             \
		testing_run_suite(&test_suite);                \
	})

test_result_t  testing_run                (test_t *);
void           testing_run_suite          (test_suite_t *);
void           testing_abort_running_test (test_result_t);
test_t        *testing_get_running_test   (void);

/* ------------------------------------------------------------- *
 *               ---------- assertions ----------                *
 * ------------------------------------------------------------- */


#define testing_assert(item)                                           (_testing_assert_((item), __FILENAME__, __LINE__))
#define testing_assert_equal(first, second)                            (_testing_assert_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_not_equal(first, second)                        (_testing_assert_not_equal_((first), second, __FILENAME__, __LINE__))
#define testing_assert_int_equal(first, second)                        (_testing_assert_int_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_int_not_equal(first, second)                    (_testing_assert_int_not_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_uint_equal(first, second)                       (_testing_assert_uint_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_uint_not_equal(first, second)                   (_testing_assert_uint_not_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_double_equal(first, second, prec)               (_testing_assert_double_equal_eps_((first), (second), (prec), __FILENAME__, __LINE__))
#define testing_assert_double_not_equal(first, second, prec)           (_testing_assert_double_not_equal_eps_((first), (second), (prec), __FILENAME__, __LINE__))
#define testing_assert_null(item)                                      (_testing_assert_null_((item), __FILENAME__, __LINE__))
#define testing_assert_not null(item)                                  (_testing_assert_not_null_((item), __FILENAME__, __LINE__))
#define testing_assert_strings_equal(first, second)                    (_testing_assert_strings_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_strings_not_equal(first, second)                (_testing_assert_strings_not_equal_((first), (second), __FILENAME__, __LINE__))
#define testing_assert_mem_equal(first, second, size)                  (_testing_assert_mem_equal_((first), (second), (size), __FILENAME__, __LINE__))
#define testing_assert_mem_not_equal(first, second, size)              (_testing_assert_mem_not_equal_((first), (second), (size), __FILENAME__, __LINE__))
#define testing_assert_structs_equal(first, second, type)              (_testing_assert_mem_equal_((first), (second), sizeof((type)), __FILENAME__, __LINE__))
#define testing_assert_structs_not_equal(first, second, type)          (_testing_assert_mem_not_equal_((first), (second), sizeof((type)), __FILENAME__, __LINE__))
#define testing_assert_arrays_equal(first, second, nitems, item_t)     (_testing_assert_arrays_equal_((first), (second), (nitems), #item_t, sizeof((item_t)), __FILENAME__, __LINE__))
#define testing_assert_arrays_not_equal(first, second, nitems, item_t) (_testing_assert_arrays_not_equal_((first), (second), (nitems), #item_t, sizeof((item_t)), __FILENAME__, __LINE__))
#define testing_assert_true(pred)                                      (_testing_assert_true_((pred), __FILENAME__, __LINE__))
#define testing_assert_false(pred)                                     (_testing_assert_false_((pred), __FILENAME__, __LINE__))
#define assertion_error(format, ...)                                   (_testing_assertion_error_((format), __FILENAME__, __LINE__, ## __VA_ARGS__))
#define testing_fail()                                                 (_testing_fail_(__FILENAME__, __LINE__))

#define testing_assert_panic(expr)                                \
	({                                                            \
		jmp_buf buf;                                              \
		jmp_buf *prev_buf = testing_panic_handler_register(&buf); \
		bool did_panic = false;                                   \
		if (setjmp(buf))                                          \
			did_panic = true;                                     \
		else                                                      \
			(expr);                                               \
		testing_panic_handler_reset(prev_buf);                    \
		if (!did_panic)                                           \
			assertion_error("did not panic");                     \
	})
     
void _testing_assert_                   (const void *, char *, size_t);
void _testing_assert_equal_             (const void *, const void *, char *, size_t);
void _testing_assert_not_equal_         (const void *, const void *, char *, size_t);
void _testing_assert_int_equal_         (long, long, char *, size_t);
void _testing_assert_int_not_equal_     (long, long, char *, size_t);
void _testing_assert_uint_equal_        (unsigned long, unsigned long, char *, size_t);
void _testing_assert_uint_not_equal_    (unsigned long, unsigned long, char *, size_t);
void _testing_ssert_double_equal_       (double, double, double eps, char *, size_t);
void _testing_assert_double_not_equal_  (double, double, double eps, char *, size_t);
void _testing_assert_null_              (const void *, char *, size_t);
void _testing_assert_not_null_          (const void *, char *, size_t);
void _testing_assert_strings_equal_     (const char *, const char *, char *, size_t);
void _testing_assert_strings_not_equal_ (const char *, const char *, char *, size_t);
void _testing_assert_mem_equal_         (const void *, const void *, size_t, char *, size_t);
void _testing_assert_mem_not_equal_     (const void *, const void *, size_t, char *, size_t);
void _testing_assert_arrays_equal_      (const void *, const void *, size_t, char *, size_t, char *, size_t);
void _testing_assert_arrays_not_equal_  (const void *, const void *, size_t, char *, size_t, char *, size_t);
void _testing_assert_true_              (bool, char *, size_t);
void _testing_assert_false_             (bool, char *, size_t);
bool _testing_check_panic_did_catch_    ();
void _testing_panic_did_catch_          ();
void _testing_assertion_error_          (char *format, char *fsource, size_t line, ...);
void _testing_fail_                     (char *, size_t);

jmp_buf  *testing_panic_handler_register (jmp_buf *new_loc);
bool      testing_panic_handler_exists   ();
void      testing_panic_handler_catch    ();
void      testing_panic_handler_reset    (jmp_buf *prev_loc);

/* ------------------------------------------------------------- *
 *               ---------- randomness ----------                *
 * ------------------------------------------------------------- */

void   testing_seed_rand         (unsigned int seed);
int    testing_rand_int          ();
int    testing_rand_int_range    (int, int);
double testing_rand_double       ();
double testing_rand_double_range (double, double);
void   testing_rand_mem          (void *, size_t);

#endif