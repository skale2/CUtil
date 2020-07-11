#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include "testing.h"

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
}

test_result_t testing_run(test_t *test)
{
	running_test = test;
	printf("- %s...\n", test->name);

	register_signal_handler();

	before_each();

	test_result_t result = setjmp(abort_test_loc);

	if (result)
		running_test->result = result;
	else
	{
		test->fn();
		running_test->result = result = TEST_RESULT_SUCCESS;
	}

	after_each();

	running_test = NULL;
	printf(result == TEST_RESULT_SUCCESS ? "  success\n\n" : "  failure\n\n");

	return result;
}

void testing_run_suite(test_suite_t *suite)
{
	test_result_t result;
	printf("\n------------ Running suite: %s ------------\n\n", suite->name);

	before_all();

	for (size_t test_idx = 0; test_idx < suite->n_tests; test_idx++)
	{
		result = testing_run(&suite->tests[test_idx]);
		if (result == TEST_RESULT_SUCCESS)
			suite->n_successful++;
		else
			suite->n_failures++;
	}

	after_all();

	size_t suite_name_l = strlen(suite->name);
	char dash_str[suite_name_l + 1];
	memset(dash_str, '-', suite_name_l);

	printf(
		"------------ Suite results: %s ------------\n\n"
		"  # successful : %zu\n"
		"  # failures   : %zu\n\n"
		"----------------------------%s-------------\n\n",
		suite->name, suite->n_successful, suite->n_failures, dash_str);
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

void _testing_assert_equal_(const void *first,
							const void *second,
							char *fsource,
							size_t line)
{
	if (first != second)
		_testing_assertion_error_("%p is not equal to %p", fsource, line, first, second);
}

void _testing_assert_not_equal_(const void *first,
								const void *second,
								char *fsource,
								size_t line)
{
	if (first == second)
		_testing_assertion_error_("%p is equal to %p", fsource, line, first, second);
}

void _testing_assert_int_equal_(long first,
								long second,
								char *fsource,
								size_t line)
{
	if (first != second)
		_testing_assertion_error_("%d is not equal to %d", fsource, line, first, second);
}

void _testing_assert_int_not_equal_(long first,
									long second,
									char *fsource,
									size_t line)
{
	if (first == second)
		_testing_assertion_error_("%d is equal to %d", fsource, line, first, second);
}

void _testing_assert_uint_equal_(unsigned long first,
								 unsigned long second,
								 char *fsource,
								 size_t line)
{
	if (first != second)
		_testing_assertion_error_("%zu is not equal to %zu", fsource, line, first, second);
}

void _testing_assert_uint_not_equal_(unsigned long first,
									 unsigned long second,
									 char *fsource,
									 size_t line)
{
	if (first == second)
		_testing_assertion_error_("%zu is equal to %zu", fsource, line, first, second);
}

void _testing_assert_double_equal_(double first,
								   double second,
								   double prec,
								   char *fsource,
								   size_t line)
{
	if (fabs(first - second) > pow(10, prec))
		_testing_assertion_error_("%d is not within %d of %d", fsource, line, first, second, prec);
}

void _testing_assert_double_not_equal_(double first,
									   double second,
									   double prec,
									   char *fsource,
									   size_t line)
{
	if (fabs(first - second) <= pow(10, prec))
		_testing_assertion_error_("%d is within %d of %d", fsource, line, first, second, prec);
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

void _testing_assert_strings_equal_(const char *first,
									const char *second,
									char *fsource,
									size_t line)
{
	if (strcmp(first, second) != 0)
		_testing_assertion_error_("%s is not equal to %s", fsource, line, first, second);
}

void _testing_assert_strings_not_equal_(const char *first,
										const char *second,
										char *fsource,
										size_t line)
{
	if (strcmp(first, second) == 0)
		assertion_error("%s is equal to %s", fsource, line, first, second);
}

void _testing_assert_mem_equal_(const void *first,
								const void *second,
								size_t size,
								char *fsource,
								size_t line)
{
	uint8_t *fref = (uint8_t *)first;
	uint8_t *sref = (uint8_t *)second;

	for (size_t i = 0; i < size; i++, fref++, sref++)
	{
		if (*fref == *sref)
			_testing_assertion_error_(
				"Bytes at %p and %p (byte %zu of buffers %p and %p respectively) are not equal",
				fsource, line, fref, sref, i, first, second);
	}
}

void _testing_assert_mem_not_equal_(const void *first,
									const void *second,
									size_t size,
									char *fsource,
									size_t line)
{
	uint8_t *fref = (uint8_t *)first;
	uint8_t *sref = (uint8_t *)second;

	for (size_t i = 0; i < size; i++, fref++, sref++)
	{
		if (*fref != *sref)
			_testing_assertion_error_(
				"Bytes at %p and %p (byte %zu of buffers %p and %p respectively) are equal",
				fsource, line, fref, sref, i, first, second);
	}
}

void _testing_assert_arrays_equal_(const void *first,
								   const void *second,
								   size_t nitems,
								   char *type_name,
								   size_t type_size,
								   char *fsource,
								   size_t line)
{
	uint8_t *fref = (uint8_t *)first;
	uint8_t *sref = (uint8_t *)second;

	for (size_t i = 0; i < nitems; i++, fref += type_size, sref += type_size)
	{
		if (memcmp(fref, sref, type_size) == 0)
			_testing_assertion_error_(
				"Items %zu of %s arrays %p and %p are not equal",
				fsource, line, fref, sref, i, type_name, first, second);
	}
}

void _testing_assert_arrays_not_equal_(const void *first,
									   const void *second,
									   size_t nitems,
									   char *type_name,
									   size_t type_size,
									   char *fsource,
									   size_t line)
{
	uint8_t *fref = (uint8_t *)first;
	uint8_t *sref = (uint8_t *)second;

	for (size_t i = 0; i < nitems; i++, fref += type_size, sref += type_size)
	{
		if (memcmp(fref, sref, type_size) != 0)
			_testing_assertion_error_(
				"Items %zu of %s arrays %p and %p are equal",
				fsource, line, fref, sref, i, type_name, first, second);
	}
}

void _testing_assert_true_(bool pred,
						   char *fsource,
						   size_t line)
{
	if (!pred)
		_testing_assertion_error_("%s is not true", fsource, line, pred ? "true" : "false");
}

void _testing_assert_false_(bool pred,
							char *fsource,
							size_t line)
{
	if (pred)
		_testing_assertion_error_("%s is not false", fsource, line, pred ? "true" : "false");
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