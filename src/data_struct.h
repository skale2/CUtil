#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "panic.h"
#include "functions.h"

#define POLYMORPHIC_DS true

#if POLYMORPHIC_DS
typedef enum ds_type
{
    DS_TYPE_ARRAY,
    DS_TYPE_ARRAY_REF,
    DS_TYPE_ARRAYLIST,
    DS_TYPE_ARRAYLIST_REF,
    DS_TYPE_LINKEDLIST,
    DS_TYPE_LINKEDLIST_REF,
    DS_TYPE_HASHMAP,
    DS_TYPE_HASHMAP_REF,
    DS_TYPE_TREEMAP,
    DS_TYPE_TREEMAP_REF,
    DS_TYPE_HASHSET,
    DS_TYPE_HASHSET_REF,
    DS_TYPE_TREESET,
    DS_TYPE_TREESET_REF
} ds_type_t;

#define is_type(ds, type_name) (ds->type == DS_TYPE_ ## type_name)
#endif

#if POLYMORPHIC_DS

/* ---------------- collection ----------------
 * A generic interface for a collection of 
 * generic items that can possibly repeat, and 
 * are in an undefined order.
 */

typedef struct collection     { ds_type_t type; } collection_t;
typedef struct collection_ref { ds_type_t type; } collection_ref_t;

size_t collection_ordered_pos(long pos, size_t collection_size);
 
size_t            collection_size               (collection_t *);               /* Returns the number of items in the collection. */
bool              collection_is_empty           (collection_t *);               /* Returns whether the collection has no items. */
bool              collection_contains           (collection_t *, void *);       /* Returns whether the collection contains the given item. */
void              collection_add                (collection_t *, void *);       /* Adds the given item to the collection. */
void             *collection_remove             (collection_t *, void *);       /* Removes the given item from the collection. If the item isn't in the collection, this is a no-op. */
void             *collection_find               (collection_t *, pred_fn_t);    /* Returns the first element that satisfies the given predicate. */
collection_t     *collection_map                (collection_t *, map_fn_t);     /* Returns a new collection comprised of the items of the collection transformed item-wise by the mapping function. */
collection_t     *collection_filter             (collection_t *, pred_fn_t);    /* Returns a new collection comprised of the items of the collection that pass through the predicate. */
void             *collection_reduce             (collection_t *, reduce_fn_t);  /* Reduces the collection into a single value by recursively applying the reduction function pair-wise until a single item remains. */
bool              collection_equal              (collection_t *, collection_t); /* Returns whether the two collections are equal. */
void              collection_free               (collection_t *);               /* Frees this collection's resources. */

collection_ref_t *collection_ref                (collection_t *);               /* Returns a reference to an initial object in the collection. */
void             *collection_ref_get_item       (collection_ref_t *);           /* Returns the item currently pointed to by the reference. */
collection_t     *collection_ref_get_collection (collection_ref_t *);           /* Returns the collection that the reference is based on. */
size_t            collection_ref_get_pos        (collection_ref_t *);           /* Returns the position the item is at inside the collection. */
bool              collection_ref_is_valid       (collection_ref_t *);           /* Returns whether the reference points to a valid item in the collection. */
bool              collection_ref_has_next       (collection_ref_t *);           /* Returns whether or not the referenced item has a successor. */
void             *collection_ref_next           (collection_ref_t *);           /* Returns whether or not the referenced item has a predecessor. */
void              collection_ref_free           (collection_ref_t *);           /* Frees the reference's resources. */

#endif /* POLYMORPHIC_DS */

/* ------------------ array -------------------
 */

#define array_ALLOW_EQ_FN_OVERLOAD true

#define array_t(type) type *

#define array(type, ...)                                \
    ({                                                  \
        type items[] = {__VA_ARGS__};                   \
        array_t(type) arr = _array_new_(                \
            (array_config_t){                           \
                .item_size = sizeof(type),              \
                .size = sizeof(items) / sizeof(type)}); \
        _array_set_all_(                                \
            arr,                                        \
            sizeof(items) / sizeof(type),               \
            (void **)&items);                           \
        arr;                                            \
    })

#define array_at(arr, pos)           ((arr)[collection_ordered_pos((pos), array_size((arr)))])
#define array_new(type, _size_, ...) ((type *)_array_new_((array_config_t){.item_size = sizeof(type), .size = _size_, __VA_ARGS__}))
#define array_size(arr)              (_array_size_((arr)))
#define array_is_empty(arr)          (array_size(arr) == 0)
#define array_get_first(arr)         (array_at((arr), 0))
#define array_get_last(arr)          (array_at((arr), array_size((arr)) - 1))
#define array_concat(arr1, arr2)     (_array_concat_((arr1), (arr2)))
#define array_pos_of(arr, item)      ({ typeof(*(arr)) i = (item); _array_pos_of_((arr), (void *)&i); })
#define array_find(arr, pred_fn)                               \
    ({                                                         \
        size_t pos = _array_find_pos_((arr), (pred_fn));       \
        if ((long)pos < 0)                                     \
            panic("No element in array satisfying predicate"); \
        (arr)[pos];                                            \
    })
#define array_map(arr, map_fn)       ((typeof(arr))_array_map_((arr), (map_fn)))
#define array_filter(arr, pred_fn)   ((typeof(arr))_array_filter_((arr), (pred_fn)))
#define array_reduce(arr, reduce_fn)                              \
    ({                                                            \
        typeof(arr) res_buf = _array_reduce_((arr), (reduce_fn)); \
        typeof(*(arr)) res = *res_buf;                            \
        free(res_buf);                                            \
        res;                                                      \
    })
#define array_slice(arr, ...)              \
    ({                                     \
        long indices[] = {__VA_ARGS__};    \
        (typeof(arr) *)_array_slice_(      \
            arr,                           \
            sizeof(indices) / sizeof(int), \
            indices);                      \
    })
#define array_unslice(slc)           ((typeof(*(slc)))_array_unslice_((slc), sizeof(**(slc))))
#define array_equal(arr1, arr2)      (_array_equal_((arr1), (arr2)))
#define array_free(arr)              (_array_free_((arr)))

typedef struct array_config
{
    size_t item_size;
    size_t size;
#if array_ALLOW_EQ_FN_OVERLOAD
    equal_fn_t equal_fn;
#endif
} array_config_t;

void    *_array_new_      (array_config_t);
size_t   _array_size_     (const void *);
void     _array_set_all_  (void *, size_t n, const void *);
void    *_array_concat_   (const void *, const void *);
size_t   _array_pos_of_   (const void *, const void *);
size_t   _array_find_pos_ (const void *, pred_fn_t);
void    *_array_map_      (const void *, map_fn_t);
void    *_array_filter_   (const void *, pred_fn_t);
void    *_array_reduce_   (const void *, reduce_fn_t);
void   **_array_slice_    (const void *, size_t n, long[n]);
void    *_array_unslice_  (const void **, size_t referenced_size);
bool     _array_equal_    (const void *, const void *);
void     _array_free_     (void *);

#define range(_start_, _stop_) \
    (range_step((_start_), (_stop_), 1))

#define range_step(_start_, _stop_, _step_)                   \
    ({                                                        \
        typeof(_start_) start = (_start_);                    \
        typeof(_start_) stop = (_stop_);                      \
        long step = (_step_);                                 \
                                                              \
        long n = (stop - start) / step +                      \
                 (int)(start % 2 && !(stop % 2) ||            \
                       stop % 2 && !(start % 2));             \
                                                              \
        array_t(typeof(_start_)) arr =                        \
            array_new(typeof(_start_), max(n, 0));            \
                                                              \
        for (typeof(_start_) i = start;                       \
             n > 0 && start < stop ? (i < stop) : (i > stop); \
             i += step)                                       \
            arr[(i - start) / step] = i;                      \
                                                              \
        arr;                                                  \
    })

/* ------------------ string -------------------
 */

#define string_t array_t(char)

#define string(chars)                                      \
    ({                                                     \
        string_t str = array_new(char, strlen(chars) + 1); \
        memcpy(str, chars, array_size(str));               \
        str;                                               \
    })

#define string_new(size)          (array_new(char, (size) + 1))
#define string_at(str, pos)       ((str)[collection_ordered_pos((pos), string_size((str)))])
#define string_size(str)          (array_size((str)) - 1)
#define string_is_empty(str)      (string_size((str)) == 0)
#define string_get_first(str)     (string_at((str), 0))
#define string_get_last(str)      (string_at((str), string_size((str)) - 1))
#define string_concat(str1, str2) (_string_concat_((str1), (str2)))
#define string_pos_of(str, ch)                  \
    ({                                          \
        size_t pos = array_pos_of(str, item);   \
        pos == string_size(str) - 1 ? -1 : pos; \
    })
#define string_find(str, ch) 1;

string_t _string_concat_ (const char *, const char *);

#if POLYMORPHIC_DS

/* ------------------- list -------------------
 * @implements collection
 *
 * A generic interface for ordered collections
 * of generic items that grow or shrink as 
 * necessary. All types that are stated as 
 * implementing list can be passed to the 
 * following list methods. 
 */

#define list_add_all(list, ...)             \
    ({                                      \
        void *items[] = {__VA_ARGS__};      \
        _list_add_all_(                     \
            list,                           \
            sizeof(items) / sizeof(void *), \
            items);                         \
    })

typedef struct list     list_t;
typedef struct list_ref list_ref_t;

size_t      list_size         (list_t *);
bool        list_is_empty     (list_t *);
bool        list_contains     (list_t *, void *);
void       *list_get_at       (list_t *, int);
void       *list_get_first    (list_t *);
void       *list_get_last	  (list_t *);
void        list_set_at       (list_t *, int, void *);
void        list_add          (list_t *, void *);
void        list_add_front    (list_t *, void *);
void        list_add_back     (list_t *, void *);
void        list_add_at       (list_t *, int, void *);
void        _list_add_all_    (list_t *, size_t n, void *[n]);
list_t     *list_concat       (list_t *, list_t *);
size_t      list_pos_of       (list_t *, void *);
void       *list_remove       (list_t *, void *);
void       *list_remove_front (list_t *);
void       *list_remove_back  (list_t *);
void       *list_remove_at    (list_t *, int);
void       *list_find         (list_t *, pred_fn_t);
list_t     *list_map          (list_t *, map_fn_t);
list_t     *list_filter       (list_t *, pred_fn_t);
void       *list_reduce       (list_t *, reduce_fn_t);
bool        list_equal        (list_t *, list_t *);
void        list_free         (list_t *);

list_ref_t *list_ref          (list_t *);
void       *list_ref_get_item (list_ref_t *);
list_t     *list_ref_get_list (list_ref_t *);
size_t      list_ref_get_pos  (list_ref_t *);
bool        list_ref_is_valid (list_ref_t *);
bool        list_ref_has_prev (list_ref_t *);
bool        list_ref_has_next (list_ref_t *);
void       *list_ref_next     (list_ref_t *);
void       *list_ref_prev     (list_ref_t *);
void        list_ref_free     (list_ref_t *);

#endif /* POLYMORPHIC_DS */

/* ------------------- arraylist ------------------
 * @implements list
 * 
 * A list implemented as a single contiguous 
 * dynamically allocated buffer, which is re-
 * allocated as necessary.
 * 
 * Has amortized O(1) lookups and assocations, and
 * sub-O(n) insertions + deletions.
 */

#define arraylist_DEFAULT_CAP 10
#define arraylist_SIZE_UP_RATIO 0.7
#define arraylist_SIZE_DOWN_RATIO 0.3
#define arraylist_ALLOW_EQ_FN_OVERLOAD true

#define arraylist_t(type)     type *
#define arraylist_ref_t(type) type *

#define arraylist(type, ...)                          \
    ({                                                \
        arraylist_t(type) list = arraylist_new(type); \
        arraylist_add_all(list, ##__VA_ARGS__);       \
        list;                                         \
    })

#define arraylist_new(type, ...) \
    ((arraylist_t(type))_arraylist_new_((arraylist_config_t){.item_size = sizeof(type), ##__VA_ARGS__}))

#define arraylist_get_config(list) \
    (_arraylist_get_config_((struct arraylist *)(list)))

#define arraylist_is_empty(list) \
    (_arraylist_is_empty_((struct arraylist *)(list)))

#define arraylist_contains(list, _item_)                         \
    ({                                                           \
        typeof((list)) item = (_item_);                          \
        _arraylist_contains_((struct arraylist *)(list), &item); \
    })

#define arraylist_size(list) \
    (_arraylist_size_((struct arraylist *)(list)))

#define arraylist_at(list, _pos_) \
    (*(typeof((list)))_arraylist_at_((struct arraylist *)(list), (_pos_)))

#define arraylist_get_first(list) \
    (*(typeof((list)))_arraylist_get_first_((struct arraylist *)(list)))

#define arraylist_get_last(list) \
    (*(typeof((list)))_arraylist_get_last_((struct arraylist *)(list)))

#define arraylist_add(list, _item_)                         \
    ({                                                      \
        typeof(*(list)) item = (_item_);                    \
        _arraylist_add_((struct arraylist *)(list), &item); \
    })

#define arraylist_add_front(list, _item_)                         \
    ({                                                            \
        typeof(*(list)) item = (_item_);                          \
        _arraylist_add_front_((struct arraylist *)(list), &item); \
    })

#define arraylist_add_back(list, _item_)                         \
    ({                                                           \
        typeof(*(list)) item = (_item_);                         \
        _arraylist_add_back_((struct arraylist *)(list), &item); \
    })

#define arraylist_add_at(list, _pos_, _item_)                           \
    ({                                                                  \
        typeof(*(list)) item = (_item_);                                \
        _arraylist_add_at_((struct arraylist *)(list), (_pos_), &item); \
    })

#define arraylist_add_all(list, ...)               \
    ({                                             \
        typeof(*(list)) items[] = {__VA_ARGS__};   \
        _arraylist_add_all_(                       \
            (struct arraylist *)(list),            \
            sizeof(items) / sizeof(typeof(*list)), \
            (void *)items);                        \
    })

#define arraylist_concat(list1, list2)                      \
    ({                                                      \
        $assert_same_type(                                  \
            (list1), (list2),                               \
            "can't concatenate arrays of different types"); \
        (typeof((list1))) _arraylist_concat_(               \
            (struct arraylist *)(list1),                    \
            (struct arraylist *)(list2));                   \
    })

#define arraylist_pos_of(list, _item_)                         \
    ({                                                         \
        typeof(*(list)) item = (_item_);                       \
        _arraylist_pos_of_((struct arraylist *)(list), &item); \
    })

#define arraylist_remove(list, _item_)          \
    ({                                          \
        typeof(*(list)) item = (_item_);        \
        *(typeof((list)))_arraylist_remove_(    \
            (struct arraylist *)(list), &item); \
    })

#define arraylist_remove_front(list) \
    (*(typeof((list)))_arraylist_remove_front_((struct arraylist *)(list)))

#define arraylist_remove_back(list) \
    (*(typeof((list)))_arraylist_remove_back_((struct arraylist *)(list)))

#define arraylist_remove_at(list, _pos_) \
    (*(typeof((list)))_arraylist_remove_at_((struct arraylist *)(list), (_pos_)))

#define arraylist_find(list, _fn_) \
    (*(typeof((list)))_arraylist_find_((struct arraylist *)(list), (_fn_)))

#define arraylist_map(list, _fn_) \
    ((typeof((list)))_arraylist_map_((struct arraylist *)(list), (_fn_)))

#define arraylist_filter(list, _fn_) \
    ((typeof((list)))_arraylist_filter_((struct arraylist *)(list), (_fn_)))

#define arraylist_reduce(list, _fn_)                 \
    ({                                               \
        typeof((list)) res_buf = _arraylist_reduce_( \
            (struct arraylist *)(list), (_fn_));     \
        typeof(*(list)) res = *res_buf;              \
        free(res_buf);                               \
        res;                                         \
    })

#define arraylist_equal(first, second, ...) \
    (_arraylist_equal_((struct arraylist *)(first), (struct arraylist *)(second), (eq_config_t){__VA_ARGS__}))

#define arraylist_free(list) \
    (_arraylist_free_((struct arraylist *)(list)))

#define arraylist_ref(list) \
    ((typeof((list)))_arraylist_ref_((struct arraylist *)(list)))

#define arraylist_ref_get_item(ref) \
    (*(typeof((ref)))_arraylist_ref_get_item_((struct arraylist_ref *)(ref)))

#define arraylist_ref_get_list(ref) \
    ((typeof((ref)))_arraylist_ref_get_list_((struct arraylist_ref *)(ref)))

#define arraylist_ref_get_pos(ref) \
    (_arraylist_ref_get_pos_((struct arraylist_ref *)(ref)))

#define arraylist_ref_is_valid(ref) \
    (_arraylist_ref_is_valid_((struct arraylist_ref *)(ref)))

#define arraylist_ref_has_prev(ref) \
    (_arraylist_ref_has_prev_((struct arraylist_ref *)(ref)))

#define arraylist_ref_has_next(ref) \
    (_arraylist_ref_has_next_((struct arraylist_ref *)(ref)))

#define arraylist_ref_next(ref) \
    (*(typeof((ref)))_arraylist_ref_next_((struct arraylist_ref *)(ref)))

#define arraylist_ref_prev(ref) \
    (*(typeof((ref)))_arraylist_ref_prev_((struct arraylist_ref *)(ref)))

#define arraylist_ref_free(ref) \
    (_arraylist_ref_free_((struct arraylist_ref *)(ref)))

struct arraylist;
struct arraylist_ref;

typedef struct arraylist_config
{
    size_t capacity;
    size_t item_size;
#if arraylist_ALLOW_EQ_FN_OVERLOAD
    equal_fn_t equal_fn;
#endif
} arraylist_config_t;

typedef struct eq_config 
{ 
    equal_fn_t eq_fn;
} eq_config_t;

struct arraylist     *_arraylist_new_          (arraylist_config_t);
arraylist_config_t    _arraylist_get_config_   (struct arraylist *);
bool                  _arraylist_is_empty_     (struct arraylist *);
bool                  _arraylist_contains_     (struct arraylist *, void *);
size_t                _arraylist_size_         (struct arraylist *);
void                 *_arraylist_at_           (struct arraylist *, int);
void                 *_arraylist_get_first_    (struct arraylist *);
void                 *_arraylist_get_last_	   (struct arraylist *);
void                  _arraylist_add_          (struct arraylist *, void *);
void                  _arraylist_add_front_    (struct arraylist *, void *);
void                  _arraylist_add_back_     (struct arraylist *, void *);
void                  _arraylist_add_at_       (struct arraylist *, long, void *);
void                  _arraylist_add_all_      (struct arraylist *, size_t, void *);
struct arraylist     *_arraylist_concat_       (struct arraylist *, struct arraylist *);
size_t                _arraylist_pos_of_       (struct arraylist *, void *);
void                 *_arraylist_remove_       (struct arraylist *, void *);
void                 *_arraylist_remove_front_ (struct arraylist *);
void                 *_arraylist_remove_back_  (struct arraylist *);
void                 *_arraylist_remove_at_    (struct arraylist *, int);
void                 *_arraylist_find_         (struct arraylist *, pred_fn_t);
struct arraylist     *_arraylist_map_          (struct arraylist *, map_fn_t);
struct arraylist     *_arraylist_filter_       (struct arraylist *, pred_fn_t);
void                 *_arraylist_reduce_       (struct arraylist *, reduce_fn_t);
bool                  _arraylist_equal_        (struct arraylist *, struct arraylist *, eq_config_t config);
void                  _arraylist_free_         (struct arraylist *);

struct arraylist_ref *_arraylist_ref_          (struct arraylist *);
void                 *_arraylist_ref_get_item_ (struct arraylist_ref *);
struct arraylist     *_arraylist_ref_get_list_ (struct arraylist_ref *);
size_t                _arraylist_ref_get_pos_  (struct arraylist_ref *);
bool                  _arraylist_ref_is_valid_ (struct arraylist_ref *);
bool                  _arraylist_ref_has_prev_ (struct arraylist_ref *);
bool                  _arraylist_ref_has_next_ (struct arraylist_ref *);
void                 *_arraylist_ref_next_     (struct arraylist_ref *);
void                 *_arraylist_ref_prev_     (struct arraylist_ref *);
void                  _arraylist_ref_free_     (struct arraylist_ref *);

/* ------------------- linkedlist -------------------
 * @implements list
 * 
 * A list implemented as a set of nodes linked to 
 * their predecessor and successor. 
 * 
 * Has O(n) lookups, O(1) associations, and ~O(1)
 * insertions and deletions.
 */

#define linkedlist_ALLOW_EQ_FN_OVERLOAD true

#define linkedlist(...)                          \
    ({                                           \
        linkedlist_t *list = linkedlist_new();   \
        linkedlist_add_all(list, ##__VA_ARGS__); \
        list;                                    \
    })

#define linkedlist_new(...) \
    (_linkedlist_new_((linkedlist_config_t){__VA_ARGS__}))

#define linkedlist_add_all(list, ...)       \
    ({                                      \
        void *items[] = {__VA_ARGS__};      \
        _linkedlist_add_all_(               \
            list,                           \
            sizeof(items) / sizeof(void *), \
            items);                         \
    })

typedef struct linkedlist     linkedlist_t;
typedef struct linkedlist_ref linkedlist_ref_t;

typedef struct linkedlist_config
{
#if linkedlist_ALLOW_EQ_FN_OVERLOAD
    equal_fn_t equal_fn;
#endif
} linkedlist_config_t;

linkedlist_t        *_linkedlist_new_            (linkedlist_config_t);
linkedlist_config_t  linkedlist_config           (linkedlist_t *);
bool                 linkedlist_is_empty         (linkedlist_t *);
bool                 linkedlist_contains         (linkedlist_t *, void *);
size_t               linkedlist_size             (linkedlist_t *);
void                *linkedlist_get_at           (linkedlist_t *, int);
void                *linkedlist_get_first        (linkedlist_t *);
void                *linkedlist_get_last	     (linkedlist_t *);
void                 linkedlist_set_at           (linkedlist_t *, int, void *);
void                 linkedlist_add              (linkedlist_t *, void *);
void                 linkedlist_add_front        (linkedlist_t *, void *);
void                 linkedlist_add_back         (linkedlist_t *, void *);
void                 linkedlist_add_at           (linkedlist_t *, int, void *);
void                 _linkedlist_add_all_        (linkedlist_t *, size_t n, void *[n]);
linkedlist_t        *linkedlist_concat           (linkedlist_t *, linkedlist_t *);
size_t               linkedlist_pos_of           (linkedlist_t *, void *);
void                *linkedlist_remove           (linkedlist_t *, void *);
void                *linkedlist_remove_front     (linkedlist_t *);
void                *linkedlist_remove_back      (linkedlist_t *);
void                *linkedlist_remove_at        (linkedlist_t *, int);
void                *linkedlist_find             (linkedlist_t *, pred_fn_t);
linkedlist_t        *linkedlist_map              (linkedlist_t *, map_fn_t);
linkedlist_t        *linkedlist_filter           (linkedlist_t *, pred_fn_t);
void                *linkedlist_reduce           (linkedlist_t *, reduce_fn_t);
linkedlist_ref_t    *linkedlist_ref_front        (linkedlist_t * list);
linkedlist_ref_t    *linkedlist_ref_back         (linkedlist_t * list);
bool                 linkedlist_equal            (linkedlist_t *, linkedlist_t *);
bool                 linkedlist_equal_item_eq_fn (linkedlist_t *, linkedlist_t *, equal_fn_t);
void                 linkedlist_free             (linkedlist_t *);

linkedlist_ref_t    *linkedlist_ref              (linkedlist_t *);
void                *linkedlist_ref_get_item     (linkedlist_ref_t *);
linkedlist_t        *linkedlist_ref_get_list     (linkedlist_ref_t *);
size_t               linkedlist_ref_get_pos      (linkedlist_ref_t *);
bool                 linkedlist_ref_is_valid     (linkedlist_ref_t *);
bool                 linkedlist_ref_has_prev     (linkedlist_ref_t *);
bool                 linkedlist_ref_has_next     (linkedlist_ref_t *);
void                *linkedlist_ref_next         (linkedlist_ref_t *);
void                *linkedlist_ref_prev         (linkedlist_ref_t *);
void                 linkedlist_ref_free         (linkedlist_ref_t *);


/* ------------------- map -------------------
 * @implements collection
 * 
 * A generic interface for a mapping between
 * a set of keys to a set of values. The map
 * is a function, so every key can have up to 
 * a single value.
 */

#define map_set_all(map, ...)                                 \
    ({                                                        \
        map_entry_t entries[] = (map_entry_t[]){__VA_ARGS__}; \
        _map_set_all_(                                        \
            map,                                              \
            sizeof(entries) / sizeof(map_entry_t),            \
            entries);                                         \
    })

typedef struct
{
    void *key;
    void *value;
} map_entry_t;

typedef struct map     map_t;
typedef struct map_ref map_ref_t;

bool              map_is_empty       (map_t *);
bool              map_contains_key   (map_t *, void *);
bool              map_contains_value (map_t *, void *);
size_t            map_size           (map_t *);
void             *map_get_at         (map_t *, void *);
void              map_set_at         (map_t *, void *, void *);
void             *map_remove_at      (map_t *, void *);
void              _map_set_all_      (map_t *, size_t n, map_entry_t[n]);
map_entry_t       map_find           (map_t *, bipred_fn_t);
struct hashset   *map_keys           (map_t *);
struct arraylist *map_values         (map_t *);
bool              map_equal          (map_t *, map_t *);
void              map_free           (map_t *);
       
map_ref_t        *map_ref            (map_t *);
void             *map_ref_get_key    (map_ref_t *);
void             *map_ref_get_value  (map_ref_t *);
map_entry_t       map_ref_get_entry  (map_ref_t *);
map_t            *map_ref_get_map    (map_ref_t *);
size_t            map_ref_get_pos    (map_ref_t *);
bool              map_ref_is_valid   (map_ref_t *);
bool              map_ref_has_next   (map_ref_t *);
map_entry_t       map_ref_next       (map_ref_t *);
void              map_ref_free       (map_ref_t *);

/* ----------------- hashmap ------------------
 * @implements map
 * 
 * An implementation of map that uses a hash
 * table to store key-value pairs. Has O(1) 
 * lookups and assocations, with unsorted keys.
 */

#define hashmap_DEFAULT_CAP 10
#define hashmap_SIZE_UP_RATIO 0.7
#define hashmap_SIZE_DOWN_RATIO 0.3
#define hashmap_ALLOW_HASH_FN_OVERLOAD true
#define hashmap_ALLOW_KEY_EQ_FN_OVERLOAD true

#define hashmap(...)                         \
    ({                                       \
        hashmap_t *map = hashmap_new();      \
        hashmap_set_all(map, ##__VA_ARGS__); \
        map;                                 \
    })

#define hashmap_new(...) \
    (_hashmap_new_((hashmap_config_t){__VA_ARGS__}))

#define hashmap_set_all(map, ...)                             \
    ({                                                        \
        map_entry_t entries[] = (map_entry_t[]){__VA_ARGS__}; \
        _hashmap_set_all_(                                    \
            map,                                              \
            sizeof(entries) / sizeof(map_entry_t),            \
            entries);                                         \
    })

typedef struct hashmap     hashmap_t;
typedef struct hashmap_ref hashmap_ref_t;

typedef struct hashmap_config
{
    size_t capacity;
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD 
    equal_fn_t key_equal_fn;
#endif
#if hashmap_ALLOW_HASH_FN_OVERLOAD 
    hash_fn_t hash_fn;
#endif
} hashmap_config_t;

hashmap_t        *_hashmap_new_          (hashmap_config_t);
hashmap_config_t  hashmap_get_config     (hashmap_t *);
bool              hashmap_is_empty       (hashmap_t *);
bool              hashmap_contains_key   (hashmap_t *, void *);
bool              hashmap_contains_value (hashmap_t *, void *);
size_t            hashmap_size           (hashmap_t *);
void             *hashmap_get_at         (hashmap_t *, void *);
void              hashmap_set_at         (hashmap_t *, void *, void *);
void             *hashmap_remove_at      (hashmap_t *, void *);
void              _hashmap_set_all_      (hashmap_t *, size_t n, map_entry_t[n]);
map_entry_t       hashmap_find           (hashmap_t *, bipred_fn_t);
struct hashset   *hashmap_keys           (hashmap_t *);
struct arraylist *hashmap_values         (hashmap_t *);
bool              hashmap_equal          (hashmap_t *, hashmap_t *);
void              hashmap_free           (hashmap_t *);
       
hashmap_ref_t    *hashmap_ref            (hashmap_t *);
void             *hashmap_ref_get_key    (hashmap_ref_t *);
void             *hashmap_ref_get_value  (hashmap_ref_t *);
map_entry_t       hashmap_ref_get_entry  (hashmap_ref_t *);
hashmap_t        *hashmap_ref_get_map    (hashmap_ref_t *);
size_t            hashmap_ref_get_pos    (hashmap_ref_t *);
bool              hashmap_ref_is_valid   (hashmap_ref_t *);
bool              hashmap_ref_has_prev   (hashmap_ref_t *);
bool              hashmap_ref_has_next   (hashmap_ref_t *);
map_entry_t       hashmap_ref_next       (hashmap_ref_t *);
map_entry_t       hashmap_ref_prev       (hashmap_ref_t *);
void              hashmap_ref_free       (hashmap_ref_t *);

/* ----------------- treemap ------------------
 * @implements map
 * 
 * An implementation of map that uses a red-
 * black tree. Has O(log n) lookups and assoc-
 * iations, with sorted keys.
 */

#define treemap_ALLOW_KEY_CMP_FN_OVERLOAD true

#define treemap(...)                         \
    ({                                       \
        treemap_t *map = treemap_new();      \
        treemap_set_all(map, ##__VA_ARGS__); \
        map;                                 \
    })

#define treemap_new(...) \
    (_treemap_new_((treemap_config_t){__VA_ARGS__}))

#define treemap_set_all(map, ...)                             \
    ({                                                        \
        map_entry_t entries[] = (map_entry_t[]){__VA_ARGS__}; \
        _treemap_set_all_(                                    \
            map,                                              \
            sizeof(entries) / sizeof(map_entry_t),            \
            entries);                                         \
    })

typedef struct treemap     treemap_t;
typedef struct treemap_ref treemap_ref_t;

typedef struct treemap_config
{
    compare_fn_t key_compare_fn;
} treemap_config_t;

treemap_t        *_treemap_new_          (treemap_config_t);
treemap_config_t  treemap_get_config     (treemap_t *);
bool              treemap_is_empty       (treemap_t *);
bool              treemap_contains_key   (treemap_t *, void *);
bool              treemap_contains_value (treemap_t *, void *);
size_t            treemap_size           (treemap_t *);
void             *treemap_get_at         (treemap_t *, void *);
void              treemap_set_at         (treemap_t *, void *, void *);
void             *treemap_remove_at      (treemap_t *, void *);
void              _treemap_set_all_      (treemap_t *, size_t n, map_entry_t[n]);
map_entry_t       treemap_find           (treemap_t *, bipred_fn_t);
struct hashset   *treemap_keys           (treemap_t *);
struct arraylist *treemap_values         (treemap_t *);
bool              treemap_equal          (treemap_t *, treemap_t *);
void              treemap_free           (treemap_t *);

treemap_ref_t    *treemap_ref            (treemap_t *);
void             *treemap_ref_get_key    (treemap_ref_t *);
void             *treemap_ref_get_value  (treemap_ref_t *);
map_entry_t       treemap_ref_get_entry  (treemap_ref_t *);
treemap_t        *treemap_ref_get_map    (treemap_ref_t *);
size_t            treemap_ref_get_pos    (treemap_ref_t *);
bool              treemap_ref_is_valid   (treemap_ref_t *);
bool              treemap_ref_has_next   (treemap_ref_t *);
map_entry_t       treemap_ref_next       (treemap_ref_t *);
void              treemap_ref_free       (treemap_ref_t *);

#if POLYMORPHIC_DS

/* -------------------- set -------------------
 * @implements collection
 * 
 * A generic interface for a collection of 
 * generic items that are all unique in the 
 * set (no repetitions allowed).
 */

#define set_add_all(set, ...)               \
    ({                                      \
        void *items[] = {__VA_ARGS__};      \
        _set_add_all_(                      \
            set,                            \
            sizeof(items) / sizeof(void *), \
            items);                         \
    })

typedef struct set     set_t;
typedef struct set_ref set_ref_t;
 
set_t     *set_new            (void);
size_t     set_size           (set_t *);
bool       set_is_empty       (set_t *);
bool       set_contains       (set_t *, void *);
void       set_add            (set_t *, void *);
void       _set_add_all_      (set_t *, size_t n, void *[n]);
void      *set_remove         (set_t *, void *);
set_t     *set_union          (set_t *, set_t *);
set_t     *set_intersection   (set_t *, set_t *);
set_t     *set_difference     (set_t *, set_t *);
bool       set_is_subset      (set_t *, set_t *subset);
void      *set_find           (set_t *, pred_fn_t);
set_t     *set_map            (set_t *, map_fn_t);
set_t     *set_filter         (set_t *, pred_fn_t);
void      *set_reduce         (set_t *, reduce_fn_t);
bool       set_equal          (set_t *, set_t *);
void       set_free           (set_t *);

set_ref_t *set_ref            (set_t *);
void      *set_ref_get_item   (set_ref_t *);
set_t     *set_ref_get_set    (set_ref_t *);
size_t     set_ref_get_pos    (set_ref_t *);
bool       set_ref_is_valid   (set_ref_t *);
bool       set_ref_has_next   (set_ref_t *);
void      *set_ref_next       (set_ref_t *);
void       set_ref_free       (set_ref_t *);

#endif

/* ------------------ hashset ------------------
 * @implements set
 * 
 * An implementation of a set using a hash table. 
 * Has O(1) lookups and insertions, with unsorted
 * keys.
 */

#define hashset(...)                         \
    ({                                       \
        hashset_t *set = hashset_new();      \
        hashset_add_all(set, ##__VA_ARGS__); \
        set;                                 \
    })

#define hashset_new(...) \
    (_hashset_new_((hashset_config_t){__VA_ARGS__}))

#define hashset_add_all(set, ...)           \
    ({                                      \
        void *items[n] = {__VA_ARGS__};     \
        _hashset_add_all_(                  \
            set,                            \
            sizeof(items) / sizeof(void *), \
            items);                         \
    })

typedef struct hashset hashset_t;
typedef struct hashset_ref hashset_ref_t;

typedef struct hashset_config
{
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
    equal_fn_t equal_fn;
#endif
#if hashmap_ALLOW_HASH_FN_OVERLOAD
    hash_fn_t hash_fn;
#endif
} hashset_config_t;
 
hashset_t        *_hashset_new_        (hashset_config_t);
hashset_config_t  hashset_get_config   (hashset_t *);
size_t            hashset_size         (hashset_t *);
bool              hashset_is_empty     (hashset_t *);
bool              hashset_contains     (hashset_t *, void *);
void              hashset_add          (hashset_t *, void *);
void              _hashset_add_all_    (hashset_t *, size_t n, void *[n]);
void             *hashset_remove       (hashset_t *, void *);
hashset_t        *hashset_union        (hashset_t *, hashset_t *);
hashset_t        *hashset_intersection (hashset_t *, hashset_t *);
hashset_t        *hashset_difference   (hashset_t *, hashset_t *);
bool              hashset_is_subset    (hashset_t *, hashset_t *subset);
void             *hashset_find         (hashset_t *, pred_fn_t);
hashset_t        *hashset_map          (hashset_t *, map_fn_t);
hashset_t        *hashset_filter       (hashset_t *, pred_fn_t);
void             *hashset_reduce       (hashset_t *, reduce_fn_t);
bool              hashset_equal        (hashset_t *, hashset_t *);
void              hashset_free         (hashset_t *);

hashset_ref_t    *hashset_ref          (hashset_t *);
void             *hashset_ref_get_item (hashset_ref_t *);
hashset_t        *hashset_ref_get_set  (hashset_ref_t *);
size_t            hashset_ref_get_pos  (hashset_ref_t *);
bool              hashset_ref_is_valid (hashset_ref_t *);
bool              hashset_ref_has_prev (hashset_ref_t *);
bool              hashset_ref_has_next (hashset_ref_t *);
void             *hashset_ref_prev     (hashset_ref_t *);
void             *hashset_ref_next     (hashset_ref_t *);
void              hashset_ref_free     (hashset_ref_t *);

/* ------------------ treeset ------------------
 * @implements set
 * 
 * An implementation of a set using a red-black
 * tree. Has O(log n) lookups and insertions, 
 * with sorted keys.
 */

#define treeset(...)                         \
    ({                                       \
        treeset_t *set = treeset_new();      \
        treeset_add_all(set, ##__VA_ARGS__); \
        set;                                 \
    })

#define treeset_new(...) \
    (_treeset_new_((treeset_config_t){__VA_ARGS__}))

#define treeset_add_all(set, ...)           \
    ({                                      \
        void *items[] = {__VA_ARGS__};      \
        treeset_add_all(                    \
            set,                            \
            sizeof(items) / sizeof(void *), \
            items);                         \
    })

typedef struct treeset     treeset_t;
typedef struct treeset_ref treeset_ref_t;

typedef struct treeset_config
{
#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
    compare_fn_t compare_fn;
#endif
} treeset_config_t;

treeset_t        *_treeset_new_        (treeset_config_t);
treeset_config_t  treeset_get_config   (treeset_t *);
size_t            treeset_size         (treeset_t *);
bool              treeset_is_empty     (treeset_t *);
bool              treeset_contains     (treeset_t *, void *);
void              treeset_add          (treeset_t *, void *);
void              _treeset_add_all_    (treeset_t *, size_t n, void *[n]);
void             *treeset_remove       (treeset_t *, void *);
treeset_t        *treeset_union        (treeset_t *, treeset_t *);
treeset_t        *treeset_intersection (treeset_t *, treeset_t *);
treeset_t        *treeset_difference   (treeset_t *, treeset_t *);
bool              treeset_is_subset    (treeset_t *, treeset_t *subset);
void             *treeset_find         (treeset_t *, pred_fn_t);
treeset_t        *treeset_map          (treeset_t *, map_fn_t);
treeset_t        *treeset_filter       (treeset_t *, pred_fn_t);
void             *treeset_reduce       (treeset_t *, reduce_fn_t);
bool              treeset_equal        (treeset_t *, treeset_t *);
void              treeset_free         (treeset_t *);

treeset_ref_t    *treeset_ref          (treeset_t *);
void             *treeset_ref_get_item (treeset_ref_t *);
treeset_t        *treeset_ref_get_set  (treeset_ref_t *);
size_t            treeset_ref_get_pos  (treeset_ref_t *);
bool              treeset_ref_is_valid (treeset_ref_t *);
bool              treeset_ref_has_next (treeset_ref_t *);
void             *treeset_ref_next     (treeset_ref_t *);
void              treeset_ref_free     (treeset_ref_t *);