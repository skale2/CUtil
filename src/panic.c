#include <stdarg.h>
#include <setjmp.h>
#include "testing.h"
#include "panic.h"

void _panic_(char *fsource, size_t line, char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (testing_panic_handler_exists())
	{
		va_end(args);
		testing_panic_handler_catch();
	}

	/* String should hold error string, source file name, and line number,
	 * along with formatting characters. */
	char error_str[strlen(format) + strlen(fsource) + 30];

	sprintf(error_str, "\tpanic @ %s[%zu] %s\n", fsource, line, format);
	vprintf(error_str, args);

	va_end(args);

	if (testing_get_running_test() != NULL)
		testing_abort_running_test(TEST_RESULT_PANIC);
	else
		exit(1);
}

panic_t new_panic_type() { 
	static panic_t counter = 0;
	return ++counter; 
}