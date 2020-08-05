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

#define FORMAT_SPEC_INT "%d"
#define FORMAT_SPEC_UINT "%u"
#define FORMAT_SPEC_CHAR "%c"
#define FORMAT_SPEC_FLOAT "%f"
#define FORMAT_SPEC_FLOAT_SCINOT "%e"
#define FORMAT_SPEC_LONG "%ld"
#define FORMAT_SPEC_ULONG "%llu"
#define FORMAT_SPEC_DOUBLE "%lf"
#define FORMAT_SPEC_DOUBLE_SCINOT "%e"
#define FORMAT_SPEC_HEX "%x"
#define FORMAT_SPEC_OCTAL "%o"
#define FORMAT_SPEC_PTR "%p"
#define FORMAT_SPEC_SIZE "%zu"
#define FORMAT_SPEC_STR "%s"

struct print_config 
{
    char *label;
    char *delim;
    char *end;
};

#define print(v, ...)                                                                             \
    ({                                                                                            \
        struct print_config config = (struct print_config){__VA_ARGS__};                          \
                                                                                                  \
        printf("%s%s",                                                                            \
               config.label == NULL ? "" : config.label,                                          \
               config.label == NULL ? "" : (config.delim == NULL ? ": " : config.delim));         \
                                                                                                  \
        printf(__builtin_choose_expr(__builtin_types_compatible_p(typeof(v), char),               \
                   FORMAT_SPEC_CHAR,                                                              \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), short) ||            \
                                     __builtin_types_compatible_p(typeof(v), int),                \
                   FORMAT_SPEC_INT,                                                               \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), unsigned short) ||   \
                                     __builtin_types_compatible_p(typeof(v), unsigned int),       \
                   FORMAT_SPEC_UINT,                                                              \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), long) ||             \
                                     __builtin_types_compatible_p(typeof(v), long long),          \
                   FORMAT_SPEC_LONG,                                                              \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), unsigned long) ||    \
                                     __builtin_types_compatible_p(typeof(v), unsigned long long), \
                   FORMAT_SPEC_ULONG,                                                             \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), float),              \
                   FORMAT_SPEC_FLOAT,                                                             \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), double),             \
                   FORMAT_SPEC_DOUBLE,                                                            \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), size_t),             \
                   FORMAT_SPEC_SIZE,                                                              \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), char []) ||          \
                                     __builtin_types_compatible_p(typeof(v), char *),             \
                   FORMAT_SPEC_STR,                                                               \
               __builtin_choose_expr(__builtin_types_compatible_p(typeof(v), void *),             \
                   FORMAT_SPEC_PTR,                                                               \
                   "\tpanic   @ " __FILE__ " (line " $string(__LINE__) ")\n"                      \
                   "\t        ! unsupported data type for generic print\n"                        \
               )))))))))), (v));                                                                  \
                                                                                                  \
        printf("%s", config.end == NULL ? "" : config.end);                                       \
    })

#define println(v, ...)          \
    ({                           \
        print(v, ##__VA_ARGS__); \
        print("\n");             \
    })


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