#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* ------------- macro helpers ------------- */

#define DEBUG_MACRO_STRINGIFY(macro) \
	(#macro)

#define DEBUG_MACRO_PRINT(macro) \
	(printf("%s\n", DEBUG_MACRO_STRINGIFY(macro)))

/* ------------- print helpers ------------ */

#define print(s, ...)           (printf(s, ##__VA_ARGS__));
#define println(s, ...)         ({ printf((s), ##__VA_ARGS__); printf("\n"); })
#define print_int(v)            (printf("%d", (v)))
#define println_int(v)          (printf("%d\n", (v)))
#define print_uint(v)           (printf("%u", (v)))
#define println_uint(v)         (printf("%u\n", (v)))
#define print_char(v)           (printf("%c", (v)))
#define println_char(v)         (printf("%c\n", (v)))
#define print_float(v)          (printf("%f", (v)))
#define println_float(v)        (printf("%f\n", (v)))
#define print_float_scinot(v)   (printf("%e", (v)))
#define println_float_scinot(v) (printf("%e\n", (v)))
#define print_long(v)           (printf("%ld", (v)))
#define println_long(v)         (printf("%ld\n", (v)))
#define print_ulong(v)          (printf("%llu", (v)))
#define println_ulong(v)        (printf("%llu\n", (v)))
#define print_double(v)         (printf("%Lf", (v)))
#define println_double(v)       (printf("%Lf\n", (v)))
#define println_int_hex(v)      (printf("%x\n", (v)))
#define print_int_octal(v)      (printf("%o", (v)))
#define println_int_octal(v)    (printf("%o\n", (v)))
#define print_bool(v)           (printf("%s", v ? "true" : "false"))
#define println_bool(v)         (printf("%s\n", v ? "true" : "false"))
#define print_ref(v)            (printf("%p", (v)))
#define println_ref(v)          (printf("%p\n", (v)))
#define print_size(v)           (printf("%zu", (v)))
#define println_size(v)         (printf("%zu\n", (v)))
#define print_str(v)            (printf("%s", (v)))
#define println_str(v)          (printf("%s\n", (v)))
#define print_nothing(v)        (printf("%n", (v)))
#define println_nothing(v)      (printf("%n\n", (v)))
#define print_member(v, m)      (printf("%p", (v)[offsetof(typeof((v)), m)]))

/* ------------ output piping ------------- */

#define SHOULD_STDOUT_DEBUG false

#define STDOUT_DEBUG_F stderr

#if SHOULD_STDOUT_DEBUG

#define printf(format, ...) fprintf(STDOUT_DEBUG_F, format, __VA_ARGS__)

#endif // SHOULD_STDOUT_DEBUG

/* ---------------- memory ---------------- */

#define SHOULD_MEMORY_DEBUG false

#define debug_mem_dump_int(start, nitems)           (debug_mem_dump((start), (nitems), int, "%d"))
#define debug_mem_dump_long(start, nitems)          (debug_mem_dump((start), (nitems), long, "%ld"))
#define debug_mem_dump_uint(start, nitems)          (debug_mem_dump((start), (nitems), unsigned int, "%i"))
#define debug_mem_dump_ulong(start, nitems)         (debug_mem_dump((start), (nitems), unsigned long, "%lu"))
#define debug_mem_dump_size(start, nitems)          (debug_mem_dump((start), (nitems), size_t, "%zu"))
#define debug_mem_dump_float(start, nitems)         (debug_mem_dump((start), (nitems), float, "%f"))
#define debug_mem_dump_double(start, nitems)        (debug_mem_dump((start), (nitems), double, "%lf"))
#define debug_mem_dump_ptr(start, nitems)           (debug_mem_dump((start), (nitems), void *, "%p"))
#define debug_mem_dump_ptr_f(start, nitems, format) (debug_mem_dump((start), (nitems), void *, format))
#define debug_mem_dump_char(start, nitems)          (debug_mem_dump((start), (nitems), char, "%c"))
#define debug_mem_dump_str(start, nitems)           (debug_mem_dump((start), (nitems), char *, "%s"))

#define debug_mem_dump(start, nitems, item_t, fmt_spec)                        \
	({                                                                         \
		printf("\nAddress        | Value"                                      \
			   "\n---------------+---------------------------------------\n"); \
                                                                               \
		for (uint8_t *ptr = (uint8_t *)start;                                  \
			 ptr < (uint8_t *)start + nitems * sizeof(item_t);                 \
			 ptr += sizeof(item_t))                                            \
		{                                                                      \
			printf("%p | ", ptr);                                              \
			printf(fmt_spec, *(item_t *)ptr);                                  \
			printf("\n");                                                      \
		}                                                                      \
	})

void debug_mem_setup(void);
void  debug_mem_reset        (void);
void *debug_mem_malloc       (size_t size, char *file, size_t line);
void *debug_mem_calloc       (size_t nitems, size_t size, char *file, size_t line);
void *debug_mem_realloc      (void *ref, size_t size, char *file, size_t line);
void  debug_mem_free         (void *ref, char *file, size_t line);
void  debug_mem_print        (void);

#if SHOULD_MEMORY_DEBUG
#define malloc(size)         (debug_mem_malloc((size), __FILE__, __LINE__))
#define calloc(nitems, size) (debug_mem_calloc((nitems), (size), __FILE__, __LINE__))
#define realloc(ref, size)   (debug_mem_realloc((ref), (size), __FILE__, __LINE__))
#define free(ref)            (debug_mem_free((ref), __FILE__, __LINE__))
#endif