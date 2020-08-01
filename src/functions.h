#pragma once

#include <string.h>
#include <stdbool.h>

#define cm ,

#define $literal(x) x

#define _(o) ((void *)(long)(o))

#define $(type, ...) \
	((type){__VA_ARGS__})

#define $box(value)                                         \
	({                                                      \
		typeof(value) *ptr = malloc(sizeof(typeof(value))); \
		*ptr = (value);                                     \
		ptr;                                                \
	})

#define $new(type, ...)                            \
	({                                             \
		type *v = (type *)calloc(1, sizeof(type)); \
		*v = (type){__VA_ARGS__};                  \
		v;                                         \
	})

#define $assert_same_type(a, b, msg) \
	_Static_assert(__builtin_types_compatible_p(typeof(a), typeof(b)), msg)

#define max(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a > _b ? _a : _b;      \
	})

#define min(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b;      \
	})

typedef bool    (*equal_fn_t)    (const void *, const void *);
typedef int     (*compare_fn_t)  (const void *, const void *);
typedef size_t  (*hash_fn_t)     (const void *);
typedef void   *(*map_fn_t)      (const void *);
typedef bool    (*pred_fn_t)     (const void *);
typedef bool    (*bipred_fn_t)   (const void *, const void *);
typedef void   *(*reduce_fn_t)   (const void *, const void *);

#define hash(...)                                      \
	({                                                 \
		const void *items[] = {__VA_ARGS__};           \
		_hash_(sizeof(items) / sizeof(void *), items); \
	})
size_t _hash_(size_t n, const void *[n]);

size_t identity_hash (const void *);
bool   streq         (const void *, const void *);
size_t strhash       (const void *);