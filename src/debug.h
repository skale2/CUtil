#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* ------------- print helpers ------------ */

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
#define print_long(v)           (printf("%lld", (v)))
#define println_long(v)         (printf("%lld\n", (v)))
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

#ifdef __GNUC__
#define print_member(v, m)      (printf("%p", (v)[offsetof(typeof((v)), m)]))
#endif

/* ------------ output piping ------------- */

#define SHOULD_STDOUT_DEBUG false

#define STDOUT_DEBUG_F stderr

#if SHOULD_STDOUT_DEBUG

#define printf(format, ...) fprintf(STDOUT_DEBUG_F, format, __VA_ARGS__)

#endif // SHOULD_STDOUT_DEBUG

/* ---------------- memory ---------------- */

#define SHOULD_MEMORY_DEBUG false

#define debug_mem_dump_int(start, nitems)    (debug_mem_dump((start), (nitems), sizeof(int), "%d"))
#define debug_mem_dump_long(start, nitems)   (debug_mem_dump((start), (nitems), sizeof(long), "%ld"))
#define debug_mem_dump_uint(start, nitems)   (debug_mem_dump((start), (nitems), sizeof(unsigned int), "%i"))
#define debug_mem_dump_ulong(start, nitems)  (debug_mem_dump((start), (nitems), sizeof(unsigned long), "%lu"))
#define debug_mem_dump_size_t(start, nitems) (debug_mem_dump((start), (nitems), sizeof(size_t), "%zu"))
#define debug_mem_dump_float(start, nitems)  (debug_mem_dump((start), (nitems), sizeof(float), "%f"))
#define debug_mem_dump_double(start, nitems) (debug_mem_dump((start), (nitems), sizeof(double), "%lf"))
#define debug_mem_dump_ptr(start, nitems)    (debug_mem_dump((start), (nitems), sizeof(void *), "%p"))
#define debug_mem_dump_char(start, nitems)   (debug_mem_dump((start), (nitems), sizeof(char), "%c"))
#define debug_mem_dump_str(start, nitems)    (debug_mem_dump((start), (nitems), sizeof(char *), "%s"))

void  debug_mem_dump         (void *start, size_t nitems, size_t item_size, char *fmt_spec);

void  debug_mem_setup        (void);
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

#endif