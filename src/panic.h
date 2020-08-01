#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define panic(format, ...) (_panic_(__FILE__, __LINE__, (format), ## __VA_ARGS__))

#define PANIC_TYPE_ALL 0;

typedef size_t panic_t;
panic_t new_panic_type();

typedef struct
{
	char *fsource;
	size_t line;
	panic_t type;
	char *reason;
} panicker;

void _panic_(char *fsource, size_t line, char *format, ...) __attribute__((noreturn));