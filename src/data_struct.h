#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stdlib.h>
#include <stdbool.h>

#define POLYMORPHIC_DS true

typedef bool    (*equal_fn_t)   (void *, void *);
typedef int     (*compare_fn_t) (void *, void *);
typedef size_t  (*hash_fn_t)    (void *);
typedef void   *(*map_fn_t)     (void *);
typedef bool    (*filter_fn_t)  (void *);
typedef void   *(*reduce_fn_t)  (void *, void *);

#if POLYMORPHIC_DS
typedef enum ds_type
{
	DS_TYPE_HASHSET,
	DS_TYPE_HASHSET_REF,
	DS_TYPE_TREESET,
	DS_TYPE_TREESET_REF,
	DS_TYPE_ARRAYLIST,
	DS_TYPE_ARRAYLIST_REF,
	DS_TYPE_LINKEDLIST,
	DS_TYPE_LINKEDLIST_REF,
	DS_TYPE_HASHMAP,
	DS_TYPE_HASHMAP_REF,
	DS_TYPE_TREEMAP,
	DS_TYPE_TREEMAP_REF
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
 
size_t            collection_size               (collection_t *);                /* Returns the number of items in the collection. */
bool              collection_is_empty           (collection_t *);                /* Returns whether the collection has no items. */
bool              collection_contains           (collection_t *, void *);        /* Returns whether the collection contains the given item. */
void              collection_add                (collection_t *, void *);        /* Adds the given item to the collection. */
void             *collection_remove             (collection_t *, void *);        /* Removes the given item from the collection. If the item isn't in the collection, this is a no-op. */
collection_t     *collection_map                (collection_t *, map_fn_t);      /* Returns a new collection comprised of the items of the collection transformed item-wise by the mapping function. */
collection_t     *collection_filter             (collection_t *, filter_fn_t);   /* Returns a new collection comprised of the items of the collection that pass through the predicate. */
void             *collection_reduce             (collection_t *, reduce_fn_t);   /* Reduces the collection into a single value by recursively applying the reduction function pair-wise until a single item remains. */
bool              collection_equal              (collection_t *, collection_t);  /* Returns whether the two collections are equal. */
void              collection_free               (collection_t *);                /* Frees this collection's resources. */

collection_ref_t *collection_ref                (collection_t *);                /* Returns a reference to an initial object in the collection. */
void             *collection_ref_get_item       (collection_ref_t *);
collection_t     *collection_ref_get_collection (collection_ref_t *);
size_t            collection_ref_get_pos        (collection_ref_t *);
bool              collection_ref_has_next       (collection_ref_t *);
void             *collection_ref_next           (collection_ref_t *);
void              collection_ref_free           (collection_ref_t *);

#endif /* POLYMORPHIC_DS */

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

typedef struct list     list_t;
typedef struct list_ref list_ref_t;

size_t      list_size         (list_t *);
bool        list_is_empty     (list_t *);
bool        list_contains     (list_t *, void *);
void       *list_get_at       (list_t *, int);
void       *list_get_first    (list_t *);
void       *list_get_last	   (list_t *);
void        list_set_at       (list_t *, int, void *);
void        list_add          (list_t *, void *);
void        list_add_front    (list_t *, void *);
void        list_add_back     (list_t *, void *);
void        list_add_at       (list_t *, int, void *);
size_t      list_pos_of       (list_t *, void *);
void       *list_remove       (list_t *, void *);
void       *list_remove_front (list_t *);
void       *list_remove_back  (list_t *);
void       *list_remove_at    (list_t *, int);
list_t     *list_map          (list_t *, map_fn_t);
list_t     *list_filter       (list_t *, filter_fn_t);
void       *list_reduce       (list_t *, reduce_fn_t);
bool        list_equal        (list_t *, list_t *);
void        list_free         (list_t *);

list_ref_t *list_ref          (list_t *);
void       *list_ref_get_item (list_ref_t *);
list_t     *list_ref_get_list (list_ref_t *);
size_t      list_ref_get_pos  (list_ref_t *);
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

#define arraylist(...)                       \
    ({                                       \
    	void *items[] = {__VA_ARGS__};       \
    	arraylist_t *list = arraylist_new(); \
    	arraylist_add_all(                   \
    		list,                            \
    		sizeof(items) / sizeof(void *),  \
    		items                            \
    	);                                   \
    	list;                                \
    })

typedef struct arraylist     arraylist_t;
typedef struct arraylist_ref arraylist_ref_t;

arraylist_t     *arraylist_new              (void);
arraylist_t     *arraylist_new_cap          (size_t);
     
#if arraylist_ALLOW_EQ_FN_OVERLOAD     
arraylist_t     *arraylist_new_eq_fn        (equal_fn_t);
arraylist_t     *arraylist_new_cap_eq_fn    (size_t, equal_fn_t);
equal_fn_t       arraylist_get_eq_fn        (arraylist_t *);
#endif

bool             arraylist_is_empty         (arraylist_t *);
bool             arraylist_contains         (arraylist_t *, void *);
size_t           arraylist_size             (arraylist_t *);
void            *arraylist_get_at           (arraylist_t *, int);
void            *arraylist_get_first        (arraylist_t *);
void            *arraylist_get_last	        (arraylist_t *);
void             arraylist_set_at           (arraylist_t *, int, void *);
void             arraylist_add              (arraylist_t *, void *);
void             arraylist_add_front        (arraylist_t *, void *);
void             arraylist_add_back         (arraylist_t *, void *);
void             arraylist_add_at           (arraylist_t *, int, void *);
void             arraylist_add_all          (arraylist_t *, size_t n, void *[n]);
size_t           arraylist_pos_of           (arraylist_t *, void *);
void            *arraylist_remove           (arraylist_t *, void *);
void            *arraylist_remove_front     (arraylist_t *);
void            *arraylist_remove_back      (arraylist_t *);
void            *arraylist_remove_at        (arraylist_t *, int);
arraylist_t     *arraylist_map              (arraylist_t *, map_fn_t);
arraylist_t     *arraylist_filter           (arraylist_t *, filter_fn_t);
void            *arraylist_reduce           (arraylist_t *, reduce_fn_t);
bool             arraylist_equal            (arraylist_t *, arraylist_t *);
bool             arraylist_equal_item_eq_fn (arraylist_t *, arraylist_t *, equal_fn_t);
void             arraylist_free             (arraylist_t *);

arraylist_ref_t *arraylist_ref              (arraylist_t *);
void            *arraylist_ref_get_item     (arraylist_ref_t *);
arraylist_t     *arraylist_ref_get_list     (arraylist_ref_t *);
size_t           arraylist_ref_get_pos      (arraylist_ref_t *);
bool             arraylist_ref_has_prev     (arraylist_ref_t *);
bool             arraylist_ref_has_next     (arraylist_ref_t *);
void            *arraylist_ref_next         (arraylist_ref_t *);
void            *arraylist_ref_prev         (arraylist_ref_t *);
void             arraylist_ref_free         (arraylist_ref_t *);

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

#define linkedlist(...)                        \
    ({                                         \
    	void *items[] = {__VA_ARGS__};         \
    	linkedlist_t *list = linkedlist_new(); \
    	linkedlist_add_all(                    \
    		list,                              \
    		sizeof(items) / sizeof(void *),    \
    		items                              \
    	);                                     \
    	list;                                  \
    })

typedef struct linkedlist     linkedlist_t;
typedef struct linkedlist_ref linkedlist_ref_t;

linkedlist_t     *linkedlist_new              (void);
    
#if linkedlist_ALLOW_EQ_FN_OVERLOAD    
linkedlist_t     *linkedlist_new_eq_fn        (equal_fn_t);
equal_fn_t        linkedlist_get_eq_fn        (linkedlist_t *);
#endif

bool              linkedlist_is_empty         (linkedlist_t *);
bool              linkedlist_contains         (linkedlist_t *, void *);
size_t            linkedlist_size             (linkedlist_t *);
void             *linkedlist_get_at           (linkedlist_t *, int);
void             *linkedlist_get_first        (linkedlist_t *);
void             *linkedlist_get_last	      (linkedlist_t *);
void              linkedlist_set_at           (linkedlist_t *, int, void *);
void              linkedlist_add              (linkedlist_t *, void *);
void              linkedlist_add_front        (linkedlist_t *, void *);
void              linkedlist_add_back         (linkedlist_t *, void *);
void              linkedlist_add_at           (linkedlist_t *, int, void *);
void              linkedlist_add_all          (linkedlist_t *, size_t n, void *[n]);
size_t            linkedlist_pos_of           (linkedlist_t *, void *);
void             *linkedlist_remove           (linkedlist_t *, void *);
void             *linkedlist_remove_front     (linkedlist_t *);
void             *linkedlist_remove_back      (linkedlist_t *);
void             *linkedlist_remove_at        (linkedlist_t *, int);
linkedlist_t     *linkedlist_map              (linkedlist_t *, map_fn_t);
linkedlist_t     *linkedlist_filter           (linkedlist_t *, filter_fn_t);
void             *linkedlist_reduce           (linkedlist_t *, reduce_fn_t);
linkedlist_ref_t *linkedlist_ref_front        (linkedlist_t * list);
linkedlist_ref_t *linkedlist_ref_back         (linkedlist_t * list);
bool              linkedlist_equal            (linkedlist_t *, linkedlist_t *);
bool              linkedlist_equal_item_eq_fn (linkedlist_t *, linkedlist_t *, equal_fn_t);
void              linkedlist_free             (linkedlist_t *);

linkedlist_ref_t *linkedlist_ref              (linkedlist_t *);
void             *linkedlist_ref_get_item     (linkedlist_ref_t *);
linkedlist_t     *linkedlist_ref_get_list     (linkedlist_ref_t *);
size_t            linkedlist_ref_get_pos      (linkedlist_ref_t *);
bool              linkedlist_ref_has_prev     (linkedlist_ref_t *);
bool              linkedlist_ref_has_next     (linkedlist_ref_t *);
void             *linkedlist_ref_next         (linkedlist_ref_t *);
void             *linkedlist_ref_prev         (linkedlist_ref_t *);
void              linkedlist_ref_free         (linkedlist_ref_t *);


/* ------------------- map -------------------
 * @implements collection
 * 
 * A generic interface for a mapping between
 * a set of keys to a set of values. The map
 * is a function, so every key can have up to 
 * a single value.
 */

typedef struct { void *key; void *value; } map_entry_t;

typedef struct map     map_t;
typedef struct map_ref map_ref_t;

bool            map_is_empty       (map_t *);
bool            map_contains_key   (map_t *, void *);
bool            map_contains_value (map_t *, void *);
size_t          map_size           (map_t *);
void           *map_get_at         (map_t *, void *);
void            map_set_at         (map_t *, void *, void *);
void           *map_remove_at      (map_t *, void *);
void            map_set_all        (map_t *, size_t n, map_entry_t[n]);
struct hashset *map_keys           (map_t *);
arraylist_t    *map_values         (map_t *);
bool            map_equal          (map_t *, map_t *);
void            map_free           (map_t *);
 
map_ref_t      *map_ref            (map_t *);
void           *map_ref_get_key    (map_ref_t *);
void           *map_ref_get_value  (map_ref_t *);
map_entry_t     map_ref_get_entry  (map_ref_t *);
map_t          *map_ref_get_map    (map_ref_t *);
size_t          map_ref_get_pos    (map_ref_t *);
bool            map_ref_has_next   (map_ref_t *);
map_entry_t     map_ref_next       (map_ref_t *);
void            map_ref_free       (map_ref_t *);

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

#define hashmap(...)                                          \
    ({                                                        \
    	map_entry_t entries[] = (map_entry_t[]){__VA_ARGS__}; \
    	hashmap_t *map = hashmap_new();                       \
    	hashmap_set_all(                                      \
    		map,                                              \
    		sizeof(entries) / sizeof(map_entry_t),            \
    		entries                                           \
    	);                                                    \
    	map;                                                  \
    })

typedef struct hashmap     hashmap_t;
typedef struct hashmap_ref hashmap_ref_t;

hashmap_t     *hashmap_new                    (void);
 
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD 
hashmap_t     *hashmap_new_key_eq_fn          (equal_fn_t);
equal_fn_t     hashmap_get_key_eq_fn          (hashmap_t *);
#endif 
 
#if hashmap_ALLOW_HASH_FN_OVERLOAD 
hashmap_t     *hashmap_new_hash_fn            (hash_fn_t);
hash_fn_t      hashmap_get_hash_fn            (hashmap_t *);
#endif

#if hashmap_ALLOW_HASH_FN_OVERLOAD && hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
hashmap_t     *hashmap_new_hash_fn_key_eq_fn  (equal_fn_t, hash_fn_t);
#endif

bool            hashmap_is_empty               (hashmap_t *);
bool            hashmap_contains_key           (hashmap_t *, void *);
bool            hashmap_contains_value         (hashmap_t *, void *);
size_t          hashmap_size                   (hashmap_t *);
void           *hashmap_get_at                 (hashmap_t *, void *);
void            hashmap_set_at                 (hashmap_t *, void *, void *);
void           *hashmap_remove_at              (hashmap_t *, void *);
void            hashmap_set_all                (hashmap_t *, size_t n, map_entry_t[n]);
struct hashset *hashmap_keys                   (hashmap_t *);
arraylist_t    *hashmap_values                 (hashmap_t *);
bool            hashmap_equal                  (hashmap_t *, hashmap_t *);
void            hashmap_free                   (hashmap_t *);
     
hashmap_ref_t  *hashmap_ref                    (hashmap_t *);
void           *hashmap_ref_get_key            (hashmap_ref_t *);
void           *hashmap_ref_get_value          (hashmap_ref_t *);
map_entry_t     hashmap_ref_get_entry          (hashmap_ref_t *);
hashmap_t      *hashmap_ref_get_map            (hashmap_ref_t *);
size_t          hashmap_ref_get_pos            (hashmap_ref_t *);
bool            hashmap_ref_has_prev           (hashmap_ref_t *);
bool            hashmap_ref_has_next           (hashmap_ref_t *);
map_entry_t     hashmap_ref_next               (hashmap_ref_t *);
map_entry_t     hashmap_ref_prev               (hashmap_ref_t *);
void            hashmap_ref_free               (hashmap_ref_t *);

/* ----------------- treemap ------------------
 * @implements map
 * 
 * An implementation of map that uses a red-
 * black tree. Has O(log n) lookups and assoc-
 * iations, with sorted keys.
 */

#define treemap_ALLOW_KEY_CMP_FN_OVERLOAD true

#define treemap(...)                                          \
    ({                                                        \
	    map_entry_t entries[] = (map_entry_t[]){__VA_ARGS__}; \
	    treemap_t *map = treemap_new();                       \
	    treemap_set_all(                                      \
	    	map,                                              \
	    	sizeof(entries) / sizeof(map_entry_t),            \
	    	entries                                           \
	    );                                                    \
	    map;                                                  \
    })

typedef struct treemap     treemap_t;
typedef struct treemap_ref treemap_ref_t;

treemap_t     *treemap_new             (void);

#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
treemap_t     *treemap_new_key_cmp_fn  (compare_fn_t);
compare_fn_t   treemap_get_key_cmp_fn  (treemap_t *);
#endif

bool            treemap_is_empty        (treemap_t *);
bool            treemap_contains_key    (treemap_t *, void *);
bool            treemap_contains_value  (treemap_t *, void *);
size_t          treemap_size            (treemap_t *);
void           *treemap_get_at          (treemap_t *, void *);
void            treemap_set_at          (treemap_t *, void *, void *);
void           *treemap_remove_at       (treemap_t *, void *);
void            treemap_set_all         (treemap_t *, size_t n, map_entry_t[n]);
struct hashset *treemap_keys            (treemap_t *);
arraylist_t    *treemap_values          (treemap_t *);
bool            treemap_equal           (treemap_t *, treemap_t *);
void            treemap_free            (treemap_t *);
  
treemap_ref_t  *treemap_ref             (treemap_t *);
void           *treemap_ref_get_key     (treemap_ref_t *);
void           *treemap_ref_get_value   (treemap_ref_t *);
map_entry_t     treemap_ref_get_entry   (treemap_ref_t *);
treemap_t      *treemap_ref_get_map     (treemap_ref_t *);
size_t          treemap_ref_get_pos     (treemap_ref_t *);
bool            treemap_ref_has_next    (treemap_ref_t *);
map_entry_t     treemap_ref_next        (treemap_ref_t *);
void            treemap_ref_free        (treemap_ref_t *);

#if POLYMORPHIC_DS

/* -------------------- set -------------------
 * @implements collection
 * 
 * A generic interface for a collection of 
 * generic items that are all unique in the 
 * set (no repetitions allowed).
 */

typedef struct set     set_t;
typedef struct set_ref set_ref_t;
 
set_t     *set_new            (void);
size_t     set_size           (set_t *);
bool       set_is_empty       (set_t *);
bool       set_contains       (set_t *, void *);
void       set_add            (set_t *, void *);
void      *set_remove         (set_t *, void *);
set_t     *set_union          (set_t *, set_t *);
set_t     *set_intersection   (set_t *, set_t *);
set_t     *set_difference     (set_t *, set_t *);
bool       set_is_subset      (set_t *, set_t *);
set_t     *set_map            (set_t *, map_fn_t);
set_t     *set_filter         (set_t *, filter_fn_t);
void      *set_reduce         (set_t *, reduce_fn_t);
bool       set_equal          (set_t *, set_t *);
void       set_free           (set_t *);

set_ref_t *set_ref            (set_t *);
void      *set_ref_get_item   (set_ref_t *);
set_t     *set_ref_get_set    (set_ref_t *);
size_t     set_ref_get_pos    (set_ref_t *);
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

#define hashset(...)                        \
    ({                                      \
    	void *items[] = {__VA_ARGS__};      \
    	hashset_t *set = hashset_new();     \
    	hashset_add_all(                    \
    		set,                            \
    		sizeof(items) / sizeof(void *), \
    		items                           \
    	);                                  \
    	set;                                \
    })

typedef struct hashset     hashset_t;
typedef struct hashset_ref hashset_ref_t;
 
hashset_t     *hashset_new                (void);

#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
hashset_t     *hashset_new_eq_fn          (equal_fn_t);
equal_fn_t     hashset_get_eq_fn          (hashset_t *);
#endif

#if hashmap_ALLOW_HASH_FN_OVERLOAD
hashset_t     *hashset_new_hash_fn        (hash_fn_t);
hash_fn_t      hashset_get_hash_fn        (hashset_t *);
#endif

#if hashmap_ALLOW_HASH_FN_OVERLOAD && hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
hashset_t     *hashset_new_hash_fn_eq_fn  (equal_fn_t, hash_fn_t);
#endif

size_t         hashset_size               (hashset_t *);
bool           hashset_is_empty           (hashset_t *);
bool           hashset_contains           (hashset_t *, void *);
void           hashset_add                (hashset_t *, void *);
void           hashset_add_all            (hashset_t *, size_t n, void *[n]);
void          *hashset_remove             (hashset_t *, void *);
hashset_t     *hashset_union              (hashset_t *, hashset_t *);
hashset_t     *hashset_intersection       (hashset_t *, hashset_t *);
hashset_t     *hashset_difference         (hashset_t *, hashset_t *);
bool           hashset_is_subset          (hashset_t *, hashset_t *);
hashset_t     *hashset_map                (hashset_t *, map_fn_t);
hashset_t     *hashset_filter             (hashset_t *, filter_fn_t);
void          *hashset_reduce             (hashset_t *, reduce_fn_t);
bool           hashset_equal              (hashset_t *, hashset_t *);
void           hashset_free               (hashset_t *);
     
hashset_ref_t *hashset_ref                (hashset_t *);
void          *hashset_ref_get_item       (hashset_ref_t *);
hashset_t     *hashset_ref_get_set        (hashset_ref_t *);
size_t         hashset_ref_get_pos        (hashset_ref_t *);
bool           hashset_ref_has_prev       (hashset_ref_t *);
bool           hashset_ref_has_next       (hashset_ref_t *);
void          *hashset_ref_prev           (hashset_ref_t *);
void          *hashset_ref_next           (hashset_ref_t *);
void           hashset_ref_free           (hashset_ref_t *);

/* ------------------ treeset ------------------
 * @implements set
 * 
 * An implementation of a set using a red-black
 * tree. Has O(log n) lookups and insertions, 
 * with sorted keys.
 */

#define treeset(...)                        \
    ({                                      \
    	void *items[] = {__VA_ARGS__};      \
    	treeset_t *set = treeset_new();     \
    	treeset_add_all(                    \
    		set,                            \
    		sizeof(items) / sizeof(void *), \
    		items                           \
    	);                                  \
    	set;                                \
    })

typedef struct treeset     treeset_t;
typedef struct treeset_ref treeset_ref_t;

treeset_t     *treeset_new             (void);

#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
treeset_t     *treeset_new_cmp_fn      (compare_fn_t);
compare_fn_t   treeset_get_cmp_fn      (treeset_t *);
#endif
 
size_t         treeset_size            (treeset_t *);
bool           treeset_is_empty        (treeset_t *);
bool           treeset_contains        (treeset_t *, void *);
void           treeset_add             (treeset_t *, void *);
void           treeset_add_all         (treeset_t *, size_t n, void *[n]);
void          *treeset_remove          (treeset_t *, void *);
treeset_t     *treeset_union           (treeset_t *, treeset_t *);
treeset_t     *treeset_intersection    (treeset_t *, treeset_t *);
treeset_t     *treeset_difference      (treeset_t *, treeset_t *);
bool           treeset_is_subset       (treeset_t *, treeset_t *);
treeset_t     *treeset_map             (treeset_t *, map_fn_t);
treeset_t     *treeset_filter          (treeset_t *, filter_fn_t);
void          *treeset_reduce          (treeset_t *, reduce_fn_t);
bool           treeset_equal           (treeset_t *, treeset_t *);
void           treeset_free            (treeset_t *);
    
treeset_ref_t *treeset_ref             (treeset_t *);
void          *treeset_ref_get_item    (treeset_ref_t *);
treeset_t     *treeset_ref_get_set     (treeset_ref_t *);
size_t         treeset_ref_get_pos     (treeset_ref_t *);
bool           treeset_ref_has_next    (treeset_ref_t *);
void          *treeset_ref_next        (treeset_ref_t *);
void           treeset_ref_free        (treeset_ref_t *);

#endif