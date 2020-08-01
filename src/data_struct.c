#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include "data_struct.h"
#include "panic.h"
#include "functions.h"
#include "debug.h"

#define INVALID_REF (-1)

size_t collection_ordered_pos(long pos, size_t collection_size)
{
	if (pos >= (long)collection_size || pos < -(long)collection_size)
		panic("Position %ld out of bounds for collection of size %d", pos, collection_size);
	if (pos < 0)
		pos = collection_size + pos;

	return pos;
}

/* ------------------------------------------------------------- */
/*                  ---------- array ----------                  */
/* ------------------------------------------------------------- */

#define header(_arr_) ((struct array_header *)((_arr_)-offsetof(struct array_header, arr)))

#define array_addr_at(buf, arr, pos) (_((intptr_t)buf + header(arr)->item_size * pos))

struct array_header
{
#if POLYMORPHIC_DS
	enum ds_type type;
#endif
#if array_ALLOW_EQ_FN_OVERLOAD
	equal_fn_t equal_fn;
#endif
	size_t item_size;
	size_t size;
	uint8_t *arr[]; // Type as byte buffer so that pointer arithmetic is easier.
};

void *_array_new_(array_config_t config)
{
	struct array_header *header = calloc(1, sizeof(struct array_header) + config.size * config.item_size);
	*header = (struct array_header)
	{
#if POLYMORPHIC_DS
		.type = DS_TYPE_ARRAY,
#endif
		.item_size = config.item_size,
		.size = config.size,
#if array_ALLOW_EQ_FN_OVERLOAD
		.equal_fn = config.equal_fn
#endif
	};

	return header->arr;
}

bool array_items_equal(const void *arr, const void *first, const void *second)
{
#if array_ALLOW_EQ_FN_OVERLOAD
	return header(arr)->equal_fn != NULL
			   ? header(arr)->equal_fn(first, second)
			   : memcmp(first, second, header(arr)->item_size) == 0;
#else
	return memcmp(first, second, header(arr)->item_size) == 0
#endif
}

size_t _array_size_(const void *arr)
{
	return header(arr)->size;
}

void _array_set_all_(void *_arr_, size_t nitems, const void *_buf_)
{
	uint8_t *arr = (uint8_t *)_arr_;
	const uint8_t *buf = (uint8_t *)_buf_;

	for (int i = 0; i < nitems && i < header(arr)->size; i++)
	{
		memcpy(array_addr_at(arr, arr, i),
			   array_addr_at(buf, arr, i),
			   header(arr)->item_size);
	}
}

void *_array_concat_(const void *_first_, const void *_second_)
{
	const uint8_t *first = (uint8_t *)_first_;
	const uint8_t *second = (uint8_t *)_second_;

	if (header(first)->item_size != header(second)->item_size)
		panic("can't concatenate arrays %p and %p with items of different sizes", first, second);

	uint8_t *new_arr = _array_new_(
		(array_config_t){
			.item_size = header(first)->item_size,
			.size = header(first)->size + header(second)->size});

	size_t new_pos = 0;

	for (int i = 0; i < header(first)->size; i++, new_pos++)
	{
		memcpy(array_addr_at(new_arr, first, new_pos),
			   array_addr_at(first, first, i),
			   header(first)->item_size);
	}

	for (int i = 0; i < header(second)->size; i++, new_pos++)
	{
		memcpy(array_addr_at(new_arr, second, new_pos),
			   array_addr_at(second, second, i),
			   header(second)->item_size);
	}

	return new_arr;
}

size_t _array_pos_of_(const void *_arr_, const void *item)
{
	const uint8_t *arr = (uint8_t *)_arr_;

	for (int i = 0; i < header(arr)->size; i++)
	{
		if (array_items_equal(arr, array_addr_at(arr, arr, i), item))
			return i;
	}

	return -1;
}

size_t _array_find_pos_(const void *_arr_, pred_fn_t pred)
{
	const uint8_t *arr = (uint8_t *)_arr_;

	for (int i = 0; i < header(arr)->size; i++)
	{
		if (pred(array_addr_at(arr, arr, i)))
			return i;
	}

	return -1;
}

void *_array_map_(const void *_arr_, map_fn_t fn)
{
	const uint8_t *arr = (uint8_t *)_arr_;

	uint8_t *new_arr = _array_new_(
		(array_config_t){
			.item_size = header(arr)->item_size,
			.size = header(arr)->size});

	for (int i = 0; i < header(arr)->size; i++)
	{
		void *result = fn(arr + header(arr)->item_size * i);
		memcpy(array_addr_at(new_arr, arr, i),
			   result,
			   header(arr)->item_size);
		free(result);
	}

	return new_arr;
}

void *_array_filter_(const void *_arr_, pred_fn_t pred)
{
	const uint8_t *arr = (uint8_t *)_arr_;
	arraylist_t *list = arraylist();

	for (int i = 0; i < header(arr)->size; i++)
	{
		if (pred(array_addr_at(arr, arr, i)))
			arraylist_add(list, array_addr_at(arr, arr, i));
	}

	uint8_t *new_arr = _array_new_(
		(array_config_t){
			.item_size = header(arr)->item_size,
			.size = arraylist_size(list)});

	for (int i = 0; i < arraylist_size(list); i++)
		memcpy(array_addr_at(new_arr, arr, i),
			   arraylist_get_at(list, i),
			   header(arr)->item_size);

	free(list);
	return new_arr;
}

void *_array_reduce_(const void *_arr_, reduce_fn_t fn)
{
	const uint8_t *arr = (uint8_t *)_arr_;
	void **work_buffer = calloc(header(arr)->size, sizeof(void *));

	for (int i = 0; i < header(arr)->size; i++)
		work_buffer[i] = array_addr_at(arr, arr, i);

	for (int stride = 1; stride < header(arr)->size; stride *= 2)
	{
		for (int i = 0; i < header(arr)->size; i += (stride * 2))
		{
			if (i + stride < header(arr)->size)
				work_buffer[i] = fn(work_buffer[i], work_buffer[i + stride]);
		}
	}

	void *result = malloc(header(arr)->item_size);
	memcpy(result, work_buffer[0], header(arr)->item_size);
	free(work_buffer);

	return result;
}

void **_array_slice_(const void *_arr_, size_t n, long indices[n])
{
	const uint8_t *arr = _arr_;
	long *default_indices = NULL;

	if (n == 0)
	{
		n = header(arr)->size;
		default_indices = calloc(n, sizeof(long));

		for (long i = 0; i < n; i++)
			default_indices[i] = i;

		indices = default_indices;
	}

	void **new_arr = _array_new_(
		(array_config_t) {
			.item_size = sizeof(void *),
			.size = n,
#if array_ALLOW_EQ_FN_OVERLOAD
			.equal_fn = header(arr)->equal_fn
#endif
		});

	for (int i = 0; i < n; i++)
	{
		new_arr[i] = array_addr_at(
			arr, arr,
			collection_ordered_pos(indices[i], header(arr)->size));
	}

	free(default_indices);
	return new_arr;
}

void *_array_unslice_(const void **arr, size_t referred_item_size)
{
	uint8_t *new_arr = _array_new_(
		(array_config_t){
			.item_size = referred_item_size,
			.size = header(arr)->size});

	for (int i = 0; i < header(arr)->size; i++)
		memcpy(array_addr_at(new_arr, new_arr, i),
			   arr[i],
			   referred_item_size);

	return new_arr;
}

bool _array_equal_(const void *_first_, const void *_second_)
{
	uint8_t *first = (uint8_t *)_first_;
	uint8_t *second = (uint8_t *)_second_;

	if (header(first)->item_size != header(second)->item_size)
		panic("can't compare arrays %p and %p with items of different sizes", first, second);
	else if (header(first)->size != header(second)->size)
		return false;

	for (int i = 0; i < header(first)->size; i++)
	{
		if (memcmp(array_addr_at(first, first, i),
				   array_addr_at(second, second, i),
				   header(first)->item_size) != 0)
			return false;
	}

	return true;
}

void _array_free_(void *arr)
{
	if (arr == NULL)
		return;
	free(header(arr));
}

string_t _string_concat_(const char *str1, const char *str2)
{
	string_t new_str = string_new(string_size(str1) + string_size(str2));
	memcpy(new_str, str1, string_size(str1));
	memcpy(new_str + string_size(str1), str2, string_size(str2));
	return new_str;
}

/* ------------------------------------------------------------- */
/*                  ---------- list ----------                   */
/* ------------------------------------------------------------- */

#if POLYMORPHIC_DS
#define list_METHOD_DISPATCH(return_t, method, ...)                            \
	({                                                                         \
		switch (list->type)                                                    \
		{                                                                      \
		case DS_TYPE_ARRAYLIST:                                                \
			return (return_t)arraylist_##method((arraylist_t *)__VA_ARGS__);   \
			break;                                                             \
                                                                               \
		case DS_TYPE_LINKEDLIST:                                               \
			return (return_t)linkedlist_##method((linkedlist_t *)__VA_ARGS__); \
			break;                                                             \
                                                                               \
		default:                                                               \
			panic("Unknown list type tag: %d", list->type);                    \
			return (return_t)0;                                                \
		}                                                                      \
	})

#define list_ref_METHOD_DISPATCH(return_t, method, ...)                                \
	({                                                                                 \
		switch (ref->type)                                                             \
		{                                                                              \
		case DS_TYPE_ARRAYLIST_REF:                                                    \
			return (return_t)arraylist_ref_##method((arraylist_ref_t *)__VA_ARGS__);   \
			break;                                                                     \
                                                                                       \
		case DS_TYPE_LINKEDLIST_REF:                                                   \
			return (return_t)linkedlist_ref_##method((linkedlist_ref_t *)__VA_ARGS__); \
			break;                                                                     \
                                                                                       \
		default:                                                                       \
			panic("Unknown list ref type tag: %d", ref->type);                         \
			return (return_t)0;                                                        \
		}                                                                              \
	})

struct list
{
	ds_type_t type;
};

struct list_ref
{
	ds_type_t type;
};

size_t list_size(list_t *list)
{
	list_METHOD_DISPATCH(size_t, size, list);
}

bool list_is_empty(list_t *list)
{
	list_METHOD_DISPATCH(bool, is_empty, list);
}

bool list_contains(list_t *list, void *item)
{
	list_METHOD_DISPATCH(bool, contains, list, item);
}

void *list_get_at(list_t *list, int pos)
{
	list_METHOD_DISPATCH(void *, get_at, list, pos);
}

void *list_get_first(list_t *list)
{
	list_METHOD_DISPATCH(void *, get_first, list);
}

void *list_get_last(list_t *list)
{
	list_METHOD_DISPATCH(void *, get_last, list);
}

void list_set_at(list_t *list, int pos, void *item)
{
	list_METHOD_DISPATCH(void, set_at, list, pos, item);
}

void list_add(list_t *list, void *item)
{
	list_METHOD_DISPATCH(void, add, list, item);
}

void list_add_front(list_t *list, void *item)
{
	list_METHOD_DISPATCH(void, add_front, list, item);
}

void list_add_back(list_t *list, void *item)
{
	list_METHOD_DISPATCH(void, add_back, list, item);
}

void list_add_at(list_t *list, int pos, void *item)
{
	list_METHOD_DISPATCH(void, add_at, list, pos, item);
}

void _list_add_all_(list_t *list, size_t nitems, void *items[nitems])
{
	switch (list->type)
	{
	case DS_TYPE_ARRAYLIST:
		_arraylist_add_all_((arraylist_t *)list, nitems, items);
		break;

	case DS_TYPE_LINKEDLIST:
		_linkedlist_add_all_((linkedlist_t *)list, nitems, items);
		break;

	default:
		panic("Unknown list type tag: %d", list->type);
	}
}

list_t *list_concat(list_t *first, list_t *second)
{
	if (is_type(first, ARRAYLIST) && is_type(second, ARRAYLIST))
		return (list_t *)arraylist_concat((arraylist_t *)first, (arraylist_t *)second);
	else if (is_type(first, LINKEDLIST) && is_type(second, LINKEDLIST))
		return (list_t *)linkedlist_concat((linkedlist_t *)first, (linkedlist_t *)second);

	list_t *new_list = (list_t *)arraylist_new(0);

	for (list_ref_t *ref = list_ref(first);
		 list_ref_is_valid(ref);
		 list_ref_next(ref))
	{
		list_add(new_list, list_ref_get_item(ref));
	}

	for (list_ref_t *ref = list_ref(second);
		 list_ref_is_valid(ref);
		 list_ref_next(ref))
	{
		list_add(new_list, list_ref_get_item(ref));
	}

	return new_list;
}

size_t list_pos_of(list_t *list, void *item)
{
	list_METHOD_DISPATCH(size_t, pos_of, list, item);
}

void *list_remove(list_t *list, void *item)
{
	list_METHOD_DISPATCH(void *, remove, list, item);
}

void *list_remove_front(list_t *list)
{
	list_METHOD_DISPATCH(void *, remove_front, list);
}

void *list_remove_back(list_t *list)
{
	list_METHOD_DISPATCH(void *, remove_back, list);
}

void *list_remove_at(list_t *list, int pos)
{
	list_METHOD_DISPATCH(void *, remove_at, list, pos);
}

void *list_find(list_t *list, pred_fn_t pred)
{
	list_METHOD_DISPATCH(void *, find, list, pred);
}

list_t *list_map(list_t *list, map_fn_t fn)
{
	list_METHOD_DISPATCH(list_t *, map, list, fn);
}

list_t *list_filter(list_t *list, pred_fn_t pred)
{
	list_METHOD_DISPATCH(list_t *, filter, list, pred);
}

void *list_reduce(list_t *list, reduce_fn_t fn)
{
	list_METHOD_DISPATCH(void *, reduce, list, fn);
}

list_ref_t *list_ref(list_t *list)
{
	list_METHOD_DISPATCH(list_ref_t *, ref, list);
}

bool list_equal(list_t *list1, list_t *list2)
{
	if (is_type(list1, ARRAYLIST) && is_type(list2, ARRAYLIST))
		return arraylist_equal((arraylist_t *)list1, (arraylist_t *)list2);
	if (is_type(list1, LINKEDLIST) && is_type(list2, LINKEDLIST))
		return linkedlist_equal((linkedlist_t *)list1, (linkedlist_t *)list2);

	if (list_size(list1) != list_size(list2))
		return false;

	for (int i = 0; i < list_size(list1); i++)
	{
		if (list_get_at(list1, i) != list_get_at(list2, i))
			return false;
	}

	return true;
}

void list_free(list_t *list)
{
	list_METHOD_DISPATCH(void, free, list);
}

void *list_ref_get_item(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(void *, get_item, ref);
}

list_t *list_ref_get_list(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(list_t *, get_list, ref);
}

size_t list_ref_get_pos(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(size_t, get_pos, ref);
}

bool list_ref_is_valid(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(bool, is_valid, ref);
}

bool list_ref_has_prev(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(bool, has_prev, ref);
}

bool list_ref_has_next(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(bool, has_next, ref);
}

void *list_ref_next(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(void *, next, ref);
}

void *list_ref_prev(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(void *, prev, ref);
}

void list_ref_free(list_ref_t *ref)
{
	list_ref_METHOD_DISPATCH(void, free, ref);
}
#endif

/* ------------------------------------------------------------- */
/*                ---------- arraylist ----------                */
/* ------------------------------------------------------------- */

struct arraylist
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
#if arraylist_ALLOW_EQ_FN_OVERLOAD
	equal_fn_t eq_fn;
#endif
	size_t capacity;
	size_t size;
	void **buffer;
};

struct arraylist_ref
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	arraylist_t *list;
	size_t pos;
};

size_t arraylist_check_pos(arraylist_t *list, int pos)
{
	if (pos >= (int)list->size || pos < (int)(-list->size))
		panic("Position %d out of bounds for list of size %d", pos, list->size);
	if (pos < 0)
		pos = list->size + pos;

	return pos;
}

void arraylist_check_size_up(arraylist_t *list)
{
	while (((double)list->size) / list->capacity >= arraylist_SIZE_UP_RATIO)
	{
		list->buffer = realloc(list->buffer, sizeof(void *) * list->capacity * 2);
		list->capacity *= 2;
	}
}

void arraylist_check_size_down(arraylist_t *list)
{
	while (list->capacity > 10 && ((double)list->size) / list->capacity <= arraylist_SIZE_DOWN_RATIO)
	{
		list->buffer = realloc(list->buffer, sizeof(void *) * list->capacity / 2);
		list->capacity /= 2;
	}
}

bool arraylist_items_equal(arraylist_t *list, void *item1, void *item2)
{
#if arraylist_ALLOW_EQ_FN_OVERLOAD
	return list->eq_fn != NULL ? list->eq_fn(item1, item2) : item1 == item2;
#else
	return item1 == item2;
#endif
}

arraylist_t *_arraylist_new_(arraylist_config_t config)
{
	size_t capacity = (long)config.capacity > 0
						  ? config.capacity
						  : arraylist_DEFAULT_CAP;

	void **buffer = (void **)calloc(capacity, sizeof(void *));

	return $new(
		arraylist_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_ARRAYLIST,
#endif
#if arraylist_ALLOW_EQ_FN_OVERLOAD
		.eq_fn = config.equal_fn,
#endif
		.capacity = capacity,
		.size = 0,
		.buffer = buffer);
}

#if arraylist_ALLOW_EQ_FN_OVERLOAD
equal_fn_t arraylist_get_eq_fn(arraylist_t *list)
{
	return list->eq_fn;
}
#endif // arraylist_ALLOW_EQ_FN_OVERLOAD

bool arraylist_is_empty(arraylist_t *list)
{
	return list->size == 0;
}

bool arraylist_contains(arraylist_t *list, void *item)
{
	for (int i = 0; i < list->size; i++)
	{
		if (arraylist_items_equal(list, list->buffer[i], item))
			return true;
	}

	return false;
}

size_t arraylist_size(arraylist_t *list)
{
	return list->size;
}

void *arraylist_get_at(arraylist_t *list, int pos)
{
	pos = arraylist_check_pos(list, pos);
	return list->buffer[pos];
}

void *arraylist_get_first(arraylist_t *list)
{
	if (list->size == 0)
		panic("Cannot access first element of empty list");

	return list->buffer[0];
}

void *arraylist_get_last(arraylist_t *list)
{
	if (list->size == 0)
		panic("Cannot access last element of empty list");

	return list->buffer[list->size - 1];
}

void arraylist_set_at(arraylist_t *list, int pos, void *item)
{
	pos = arraylist_check_pos(list, pos);
	list->buffer[pos] = item;
}

void arraylist_add(arraylist_t *list, void *item)
{
	arraylist_add_back(list, item);
}

void arraylist_add_front(arraylist_t *list, void *item)
{
	arraylist_check_size_up(list);

	void **new_buffer = calloc(list->capacity, sizeof(void *));
	memcpy(new_buffer + 1, list->buffer, sizeof(void *) * (list->size));
	free(list->buffer);

	list->size++;
	list->buffer = new_buffer;
	list->buffer[0] = item;
}

void arraylist_add_back(arraylist_t *list, void *item)
{
	arraylist_check_size_up(list);
	list->buffer[list->size++] = item;
}

void arraylist_add_at(arraylist_t *list, int pos, void *item)
{
	arraylist_check_size_up(list);
	pos = arraylist_check_pos(list, pos);

	memcpy(list->buffer + pos + 1, list->buffer + pos, sizeof(void *) * (list->size - pos));
	list->buffer[pos] = item;
	list->size++;
}

void _arraylist_add_all_(arraylist_t *list, size_t nitems, void *items[nitems])
{
	/* Extend list to the requisite capacity to reduce resizes. */
	if (list->size + nitems / list->capacity >= arraylist_SIZE_UP_RATIO)
	{
		list->buffer = realloc(list->buffer, (list->size + nitems) * 1 / arraylist_SIZE_UP_RATIO);
		list->capacity = (list->size + nitems) * 1 / arraylist_SIZE_UP_RATIO;
	}

	for (size_t i = 0; i < nitems; i++)
		arraylist_add(list, items[i]);
}

arraylist_t *arraylist_concat(arraylist_t *first, arraylist_t *second)
{
	arraylist_t *new_list = arraylist_new(0);

	for (arraylist_ref_t *ref = arraylist_ref(first);
		 arraylist_ref_is_valid(ref);
		 arraylist_ref_next(ref))
	{
		arraylist_add(new_list, arraylist_ref_get_item(ref));
	}

	for (arraylist_ref_t *ref = arraylist_ref(second);
		 arraylist_ref_is_valid(ref);
		 arraylist_ref_next(ref))
	{
		arraylist_add(new_list, arraylist_ref_get_item(ref));
	}

	return new_list;
}

size_t arraylist_pos_of(arraylist_t *list, void *item)
{
	for (int i = 0; i < list->size; i++)
	{
		if (arraylist_items_equal(list, list->buffer[i], item))
			return i;
	}

	return -1;
}

void *arraylist_remove(arraylist_t *list, void *item)
{
	size_t pos = arraylist_pos_of(list, item);
	return (int)pos >= 0 ? arraylist_remove_at(list, pos) : NULL;
}

void *arraylist_remove_front(arraylist_t *list)
{
	if (list->size == 0)
		return NULL;

	arraylist_check_size_down(list);

	void *item = list->buffer[0];

	void **new_buffer = calloc(list->capacity, sizeof(void *));
	memcpy(new_buffer, list->buffer + 1, sizeof(void *) * (list->size));
	free(list->buffer);

	list->buffer = new_buffer;
	list->size--;

	return item;
}

void *arraylist_remove_back(arraylist_t *list)
{
	if (list->size == 0)
		return NULL;

	arraylist_check_size_down(list);

	void *item = list->buffer[list->size - 1];
	list->buffer[--list->size] = NULL;
	return item;
}

void *arraylist_remove_at(arraylist_t *list, int pos)
{
	arraylist_check_size_down(list);
	pos = arraylist_check_pos(list, pos);

	void *item = list->buffer[pos];
	memcpy(list->buffer + pos, list->buffer + pos + 1, sizeof(void *) * (list->size - pos - 1));
	list->size--;
	return item;
}

void *arraylist_find(arraylist_t *list, pred_fn_t pred)
{
	for (int i = 0; i < list->size; i++)
	{
		if (pred(list->buffer[i]))
			return list->buffer[i];
	}

	return NULL;
}

arraylist_t *arraylist_map(arraylist_t *list, map_fn_t fn)
{
	arraylist_t *new_list = arraylist_new(.capacity = list->capacity);

	for (int i = 0; i < list->size; i++)
		new_list->buffer[i] = fn(list->buffer[i]);

	new_list->size = list->size;
	return new_list;
}

arraylist_t *arraylist_filter(arraylist_t *list, pred_fn_t pred)
{
	arraylist_t *new_list = arraylist_new();
	for (int i = 0; i < list->size; i++)
	{
		if (pred(list->buffer[i]))
			arraylist_add(new_list, list->buffer[i]);
	}

	return new_list;
}

void *arraylist_reduce(arraylist_t *list, reduce_fn_t fn)
{
	void **work_buffer = calloc(list->size, sizeof(void *));
	memcpy(work_buffer, list->buffer, sizeof(void *) * list->size);

	for (int stride = 1; stride < list->size; stride *= 2)
	{
		for (int i = 0; i < list->size; i += (stride * 2))
		{
			if (i + stride < list->size)
				work_buffer[i] = fn(work_buffer[i], work_buffer[i + stride]);
		}
	}

	void *result = work_buffer[0];
	free(work_buffer);
	return result;
}

bool _arraylist_equal_(arraylist_t *list1, arraylist_t *list2, eq_config_t config)
{
	if (list1->size != list2->size)
		return false;

	for (int i = 0; i < list1->size; i++)
	{
		if ((config.eq_fn == NULL && list1->buffer[i] != list2->buffer[i]) ||
			(config.eq_fn != NULL && !config.eq_fn(list1->buffer[i], list2->buffer[i])))
			return false;
	}

	return true;
}

arraylist_ref_t *arraylist_ref(arraylist_t *list)
{
	if (list->size == 0)
		return NULL;

	return $new(
		arraylist_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_ARRAYLIST_REF,
#endif
		.list = list,
		.pos = 0);
}

void arraylist_free(arraylist_t *list)
{
	free(list->buffer);
	free(list);
}

void *arraylist_ref_get_item(arraylist_ref_t *ref)
{
	if (!arraylist_ref_is_valid(ref))
	{
		panic("Reference is out of bounds");
		return NULL;
	}
	size_t pos = arraylist_check_pos(ref->list, ref->pos);
	return ref->list->buffer[pos];
}

arraylist_t *arraylist_ref_get_list(arraylist_ref_t *ref)
{
	return ref->list;
}

size_t arraylist_ref_get_pos(arraylist_ref_t *ref)
{
	if (!arraylist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return arraylist_check_pos(ref->list, ref->pos);
}

bool arraylist_ref_has_prev(arraylist_ref_t *ref)
{
	if (!arraylist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos > 0;
}

bool arraylist_ref_is_valid(arraylist_ref_t *ref)
{
	return ref->pos != INVALID_REF;
}

bool arraylist_ref_has_next(arraylist_ref_t *ref)
{
	if (!arraylist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos < ref->list->size - 1;
}

void *arraylist_ref_next(arraylist_ref_t *ref)
{
	if (!arraylist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!arraylist_ref_has_next(ref))
	{
		ref->pos = INVALID_REF;
		return NULL;
	}
	return ref->list->buffer[++ref->pos];
}

void *arraylist_ref_prev(arraylist_ref_t *ref)
{
	if (!arraylist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!arraylist_ref_has_prev(ref))
	{
		ref->pos = INVALID_REF;
		return NULL;
	}
	return ref->list->buffer[--ref->pos];
}

void arraylist_ref_free(arraylist_ref_t *ref)
{
	free(ref);
}

/* ------------------------------------------------------------- */
/*                ---------- linkedlist ----------               */
/* ------------------------------------------------------------- */

struct linkedlist_node
{
	void *item;
	struct linkedlist_node *prev;
	struct linkedlist_node *next;
};

struct linkedlist
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
#if linkedlist_ALLOW_EQ_FN_OVERLOAD
	equal_fn_t eq_fn;
#endif
	size_t size;
	struct linkedlist_node *first;
	struct linkedlist_node *last;
};

struct linkedlist_ref
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	linkedlist_t *list;
	size_t pos;
	struct linkedlist_node *node;
};

bool linkedlist_items_equal(linkedlist_t *list, void *item1, void *item2)
{
#if linkedlist_ALLOW_EQ_FN_OVERLOAD
	return list->eq_fn != NULL ? list->eq_fn(item1, item2) : item1 == item2;
#else
	return item1 == item2;
#endif
}

linkedlist_t *_linkedlist_new_(linkedlist_config_t config)
{
	return $new(
		linkedlist_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_LINKEDLIST_REF,
#endif
#if linkedlist_ALLOW_EQ_FN_OVERLOAD
		.eq_fn = config.equal_fn,
#endif
		.size = 0,
		.first = NULL,
		.last = NULL);
}

#if linkedlist_ALLOW_EQ_FN_OVERLOAD
equal_fn_t linkedlist_get_eq_fn(linkedlist_t *list)
{
	return list->eq_fn;
}
#endif // linkedlist_ALLOW_EQ_FN_OVERLOAD

size_t linkedlist_check_pos(linkedlist_t *list, int pos)
{
	if (pos >= (int)list->size || pos < (int)(-list->size))
		panic("Position %d out of bounds for list of size %d", pos, list->size);
	if (pos < 0)
		pos = list->size + pos;

	return pos;
}

bool linkedlist_is_empty(linkedlist_t *list)
{
	return list->size == 0;
}

bool linkedlist_contains(linkedlist_t *list, void *item)
{
	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next)
	{
		if (linkedlist_items_equal(list, node->item, item))
			return true;
	}

	return false;
}

size_t linkedlist_size(linkedlist_t *list)
{
	return list->size;
}

void *linkedlist_get_at(linkedlist_t *list, int pos)
{
	pos = linkedlist_check_pos(list, pos);
	struct linkedlist_node *node = list->first;
	for (int i = 0; i < pos; node = node->next, i++)
		;
	return node->item;
}

void *linkedlist_get_first(linkedlist_t *list)
{
	if (list->size == 0)
		panic("Cannot access first element of empty list");

	return list->first->item;
}

void *linkedlist_get_last(linkedlist_t *list)
{
	if (list->size == 0)
		panic("Cannot access last element of empty list");

	return list->last->item;
}

void linkedlist_set_at(linkedlist_t *list, int pos, void *item)
{
	pos = linkedlist_check_pos(list, pos);

	struct linkedlist_node *node = list->first;
	for (int i = 0; i < pos; node = node->next, i++)
		;
	node->item = item;
}

void linkedlist_add(linkedlist_t *list, void *item)
{
	linkedlist_add_back(list, item);
}

void linkedlist_add_front(linkedlist_t *list, void *item)
{
	struct linkedlist_node *node = $new(
		struct linkedlist_node,
		.item = item,
		.prev = NULL,
		.next = list->first);

	if (list->size == 0)
		list->first = list->last = node;
	else
	{
		list->first->prev = node;
		list->first = node;
	}

	list->size++;
}

void linkedlist_add_back(linkedlist_t *list, void *item)
{
	struct linkedlist_node *node = $new(
		struct linkedlist_node,
		.item = item,
		.prev = list->last,
		.next = NULL);

	if (list->size == 0)
		list->first = list->last = node;
	else
	{
		list->last->next = node;
		list->last = node;
	}

	list->size++;
}

void linkedlist_add_at(linkedlist_t *list, int pos, void *item)
{
	pos = linkedlist_check_pos(list, pos);

	struct linkedlist_node *node = list->first;
	for (int i = 0; i < pos; node = node->next, i++)
		;

	struct linkedlist_node *new_node = $new(
		struct linkedlist_node,
		.item = item,
		.prev = node->prev,
		.next = node);

	node->prev->next = new_node;
	node->prev = new_node;
	list->size++;
}

void _linkedlist_add_all_(linkedlist_t *list, size_t nitems, void *items[nitems])
{
	for (size_t i = 0; i < nitems; i++)
		linkedlist_add(list, items[i]);
}

linkedlist_t *linkedlist_concat(linkedlist_t *first, linkedlist_t *second)
{
	linkedlist_t *new_list = linkedlist_new(0);

	for (linkedlist_ref_t *ref = linkedlist_ref(first);
		 linkedlist_ref_is_valid(ref);
		 linkedlist_ref_next(ref))
	{
		linkedlist_add(new_list, linkedlist_ref_get_item(ref));
	}

	for (linkedlist_ref_t *ref = linkedlist_ref(second);
		 linkedlist_ref_is_valid(ref);
		 linkedlist_ref_next(ref))
	{
		linkedlist_add(new_list, linkedlist_ref_get_item(ref));
	}

	return new_list;
}

size_t linkedlist_pos_of(linkedlist_t *list, void *item)
{
	int i = 0;
	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next, i++)
	{
		if (linkedlist_items_equal(list, node->item, item))
			return i;
	}

	return -1;
}

void *linkedlist_remove(linkedlist_t *list, void *item)
{
	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next)
	{
		if (linkedlist_items_equal(list, node->item, item))
		{
			if (node->next != NULL)
				node->next->prev = node->prev;
			if (node->prev != NULL)
				node->prev->next = node->next;
			if (list->first == node)
				list->first = node->next;
			if (list->last == node)
				list->last = node->prev;

			list->size--;
			void *item = node->item;
			free(node);

			return node->item;
		}
	}

	return NULL;
}

void *linkedlist_remove_front(linkedlist_t *list)
{
	if (list->size == 0)
		return NULL;

	struct linkedlist_node *first = list->first;

	if (first->next != NULL)
		first->next->prev = NULL;
	if ((list->first = first->next) == NULL)
		list->last = NULL;

	list->size--;
	void *item = first->item;
	free(first);

	return item;
}

void *linkedlist_remove_back(linkedlist_t *list)
{
	if (list->size == 0)
		return NULL;

	struct linkedlist_node *last = list->last;

	if (last->prev != NULL)
		last->prev->next = NULL;
	if ((list->last = last->prev) == NULL)
		list->first = NULL;

	list->size--;
	void *item = last->item;
	free(last);

	return item;
}

void *linkedlist_remove_at(linkedlist_t *list, int pos)
{
	pos = linkedlist_check_pos(list, pos);

	struct linkedlist_node *node = list->first;
	for (int i = 0; i < pos; node = node->next, i++)
		;

	if (node->prev != NULL)
		node->prev->next = node->next;
	if (node->next != NULL)
		node->next->prev = node->prev;
	if (list->first == node)
		list->first = node->next;
	if (list->last == node)
		list->last = node->prev;

	list->size--;
	void *item = node->item;
	free(node);

	return item;
}

void *linkedlist_find(linkedlist_t *list, pred_fn_t pred)
{
	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next)
	{
		if (pred(node->item))
			return node->item;
	}

	return NULL;
}

linkedlist_t *linkedlist_map(linkedlist_t *list, map_fn_t fn)
{
	linkedlist_t *new_list = linkedlist_new();

	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next)
		linkedlist_add_back(new_list, fn(node->item));

	return new_list;
}

linkedlist_t *linkedlist_filter(linkedlist_t *list, pred_fn_t pred)
{
	linkedlist_t *new_list = linkedlist_new();

	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next)
	{
		if (pred(node->item))
			linkedlist_add_back(new_list, node->item);
	}

	return new_list;
}

void *linkedlist_reduce(linkedlist_t *list, reduce_fn_t fn)
{
	void **work_buffer = calloc(list->size, sizeof(void *));

	int i = 0;
	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next, i++)
		work_buffer[i] = node->item;

	for (int stride = 1; stride < list->size; stride *= 2)
	{
		for (int i = 0; i < list->size; i += (stride * 2))
		{
			if (i + stride < list->size)
				work_buffer[i] = fn(work_buffer[i], work_buffer[i + stride]);
		}
	}

	void *result = work_buffer[0];
	free(work_buffer);
	return result;
}

bool linkedlist_equal(linkedlist_t *list1, linkedlist_t *list2)
{
	if (list1->size != list2->size)
		return false;

	struct linkedlist_node *node1 = list1->first;
	struct linkedlist_node *node2 = list2->first;

	for (; node1 != NULL && node2 != NULL; node1 = node1->next, node2 = node2->next)
	{
		if (node1->item != node2->item)
			return false;
	}

	return true;
}

bool linkedlist_equal_item_eq_fn(linkedlist_t *list1, linkedlist_t *list2, equal_fn_t eq_fn)
{
	if (list1->size != list2->size)
		return false;

	struct linkedlist_node *node1 = list1->first;
	struct linkedlist_node *node2 = list2->first;

	for (; node1 != NULL && node2 != NULL; node1 = node1->next, node2 = node2->next)
	{
		if (!eq_fn(node1->item, node2->item))
			return false;
	}

	return true;
}

linkedlist_ref_t *linkedlist_ref(linkedlist_t *list)
{
	return linkedlist_ref_front(list);
}

linkedlist_ref_t *linkedlist_ref_front(linkedlist_t *list)
{
	if (list->size == 0)
		return NULL;

	return $new(
		linkedlist_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_LINKEDLIST_REF,
#endif
		.list = list,
		.pos = 0,
		.node = list->first);
}

linkedlist_ref_t *linkedlist_ref_back(linkedlist_t *list)
{
	if (list->size == 0)
		return NULL;

	return $new(
		linkedlist_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_LINKEDLIST_REF,
#endif
		.list = list,
		.pos = 0,
		.node = list->last);
}

void linkedlist_free(linkedlist_t *list)
{
	for (struct linkedlist_node *node = list->first; node != NULL; node = node->next)
		free(node);
	free(list);
}

void *linkedlist_ref_get_item(linkedlist_ref_t *ref)
{
	if (!linkedlist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->node->item;
}

linkedlist_t *linkedlist_ref_get_list(linkedlist_ref_t *ref)
{
	return ref->list;
}

size_t linkedlist_ref_get_pos(linkedlist_ref_t *ref)
{
	if (!linkedlist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos;
}

bool linkedlist_ref_is_valid(linkedlist_ref_t *ref)
{
	return ref->pos != INVALID_REF;
}

bool linkedlist_ref_has_prev(linkedlist_ref_t *ref)
{
	if (!linkedlist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos > 0;
}

bool linkedlist_ref_has_next(linkedlist_ref_t *ref)
{
	if (!linkedlist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos < ref->list->size - 1;
}

void *linkedlist_ref_next(linkedlist_ref_t *ref)
{
	if (!linkedlist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!linkedlist_ref_has_next(ref))
	{
		ref->pos = INVALID_REF;
		return NULL;
	}
	ref->node = ref->node->next;
	ref->pos++;
	return ref->node->item;
}

void *linkedlist_ref_prev(linkedlist_ref_t *ref)
{
	if (!linkedlist_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!linkedlist_ref_has_prev(ref))
	{
		ref->pos = INVALID_REF;
		return NULL;
	}
	ref->node = ref->node->prev;
	ref->pos--;
	return ref->node->item;
}

void linkedlist_ref_free(linkedlist_ref_t *ref)
{
	free(ref);
}

/* ------------------------------------------------------------- */
/*                    ---------- map ----------                  */
/* ------------------------------------------------------------- */

#if POLYMORPHIC_DS
#define map_METHOD_DISPATCH(return_t, method, ...)                       \
	({                                                                   \
		switch (map->type)                                               \
		{                                                                \
		case DS_TYPE_HASHMAP:                                            \
			return (return_t)hashmap_##method((hashmap_t *)__VA_ARGS__); \
			break;                                                       \
                                                                         \
		case DS_TYPE_TREEMAP:                                            \
			return (return_t)treemap_##method((treemap_t *)__VA_ARGS__); \
			break;                                                       \
                                                                         \
		default:                                                         \
			panic("Unknown map type tag: %d", map->type);                \
			return (return_t)0;                                          \
		}                                                                \
	})

#define map_ref_METHOD_DISPATCH(return_t, method, ...)                           \
	({                                                                           \
		switch (ref->type)                                                       \
		{                                                                        \
		case DS_TYPE_HASHMAP_REF:                                                \
			return (return_t)hashmap_ref_##method((hashmap_ref_t *)__VA_ARGS__); \
			break;                                                               \
                                                                                 \
		case DS_TYPE_TREEMAP_REF:                                                \
			return (return_t)treemap_ref_##method((treemap_ref_t *)__VA_ARGS__); \
			break;                                                               \
                                                                                 \
		default:                                                                 \
			panic("Unknown map ref type tag: %d", ref->type);                    \
			return (return_t)0;                                                  \
		}                                                                        \
	})

struct map
{
	ds_type_t type;
};

struct map_ref
{
	ds_type_t type;
};

bool map_is_empty(map_t *map)
{
	map_METHOD_DISPATCH(bool, is_empty, map);
}

bool map_contains_key(map_t *map, void *key)
{
	map_METHOD_DISPATCH(bool, contains_key, map, key);
}

bool map_contains_value(map_t *map, void *value)
{
	map_METHOD_DISPATCH(bool, contains_value, map, value);
}

size_t map_size(map_t *map)
{
	map_METHOD_DISPATCH(size_t, size, map);
}

void *map_get_at(map_t *map, void *key)
{
	map_METHOD_DISPATCH(void *, get_at, map, key);
}

void map_set_at(map_t *map, void *key, void *value)
{
	map_METHOD_DISPATCH(void, set_at, map, key, value);
}

void _map_set_all_(map_t *map, size_t nitems, map_entry_t items[nitems])
{
	switch (map->type)
	{
	case DS_TYPE_HASHMAP:
		_hashmap_set_all_((hashmap_t *)map, nitems, items);
		break;

	case DS_TYPE_TREEMAP:
		_treemap_set_all_((treemap_t *)map, nitems, items);
		break;

	default:
		panic("Unknown map type tag: %d", map->type);
	}
}

void *map_remove_at(map_t *map, void *key)
{
	map_METHOD_DISPATCH(void *, remove_at, map, key);
}

hashset_t *map_keys(map_t *map)
{
	map_METHOD_DISPATCH(hashset_t *, keys, map);
}

arraylist_t *map_values(map_t *map)
{
	map_METHOD_DISPATCH(arraylist_t *, values, map);
}

map_ref_t *map_ref(map_t *map)
{
	map_METHOD_DISPATCH(map_ref_t *, ref, map);
}

bool map_equal(map_t *map1, map_t *map2)
{
	if (is_type(map1, HASHMAP) && is_type(map2, HASHMAP))
		return hashmap_equal((hashmap_t *)map1, (hashmap_t *)map2);
	else if (is_type(map1, TREEMAP) && is_type(map2, TREEMAP))
		return treemap_equal((treemap_t *)map1, (treemap_t *)map2);

	for (map_ref_t *ref = map_ref(map1);
		 map_ref_has_next(ref);
		 map_ref_next(ref))
	{
		map_entry_t entry = map_ref_get_entry(ref);
		if (!map_contains_key(map2, entry.key) || map_get_at(map2, entry.key) != entry.value)
			return false;
	}

	for (map_ref_t *ref = map_ref(map2);
		 map_ref_has_next(ref);
		 map_ref_next(ref))
	{
		map_entry_t entry = map_ref_get_entry(ref);
		if (!map_contains_key(map1, entry.key) || map_get_at(map1, entry.key) != entry.value)
			return false;
	}

	return true;
}

void map_free(map_t *map)
{
	map_METHOD_DISPATCH(void, free, map);
}

void *map_ref_get_key(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(void *, get_key, ref);
}

void *map_ref_get_value(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(void *, get_value, ref);
}

map_entry_t map_ref_get_entry(map_ref_t *ref)
{
	/* Can't use macro, since return type isn't pointer
	 * or arithmetic type. */

	switch (ref->type)
	{
	case DS_TYPE_HASHMAP_REF:
		return hashmap_ref_get_entry((hashmap_ref_t *)ref);
		break;

	case DS_TYPE_TREEMAP_REF:
		return treemap_ref_get_entry((treemap_ref_t *)ref);
		break;

	default:
		panic("Unknown map ref type tag: %d", ref->type);
		return (map_entry_t){NULL, NULL};
	}
}

map_t *map_ref_get_map(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(map_t *, get_map, ref);
}

size_t map_ref_get_pos(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(size_t, get_pos, ref);
}

bool map_ref_is_valid(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(bool, is_valid, ref);
}

bool map_ref_has_next(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(bool, has_next, ref);
}

map_entry_t map_ref_next(map_ref_t *ref)
{
	/* Can't use macro, since return type isn't pointer
	 * or arithmetic type. */

	switch (ref->type)
	{
	case DS_TYPE_HASHMAP_REF:
		return hashmap_ref_next((hashmap_ref_t *)ref);
		break;

	case DS_TYPE_TREEMAP_REF:
		return treemap_ref_next((treemap_ref_t *)ref);
		break;

	default:
		panic("Unknown map ref type tag: %d", ref->type);
		return (map_entry_t){NULL, NULL};
	}
}

void map_ref_free(map_ref_t *ref)
{
	map_ref_METHOD_DISPATCH(void, free, ref);
}
#endif

/* ------------------------------------------------------------- */
/*                  ---------- hashmap ----------                */
/* ------------------------------------------------------------- */

struct hashmap_entry
{
	void *key;
	void *value;
	struct hashmap_entry *next;
	struct hashmap_entry *prev_entry;
	struct hashmap_entry *next_entry;
};

struct hashmap
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
	equal_fn_t key_eq_fn;
#endif
#if hashmap_ALLOW_HASH_FN_OVERLOAD
	hash_fn_t hash_fn;
#endif
	size_t capacity;
	size_t size;
	struct hashmap_entry *first_entry;
	struct hashmap_entry *last_entry;
	struct hashmap_entry **buffer;
};

struct hashmap_ref
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	hashmap_t *map;
	size_t pos;
	struct hashmap_entry *entry;
};

bool hashmap_keys_eq(hashmap_t *map, void *key1, void *key2)
{
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
	return map->key_eq_fn != NULL ? map->key_eq_fn(key1, key2) : key1 == key2;
#else
	return key1 == key2;
#endif
}

size_t hashmap_pos_by_capacity(hashmap_t *map, void *key, size_t capacity)
{
#if hashmap_ALLOW_HASH_FN_OVERLOAD
	return (map->hash_fn != NULL ? map->hash_fn(key) : (size_t)key) % capacity;
#else
	return (size_t)key % capacity;
#endif
}

size_t hashmap_pos(hashmap_t *map, void *key)
{
#if hashmap_ALLOW_HASH_FN_OVERLOAD
	return (map->hash_fn != NULL ? map->hash_fn(key) : (size_t)key) % map->capacity;
#else
	return (size_t)key % map->capacity;
#endif
}

void hashmap_rehash(hashmap_t *map, size_t new_capacity)
{
	struct hashmap_entry **new_buffer = calloc(new_capacity, sizeof(void *));

	for (struct hashmap_entry *entry = map->first_entry;
		 entry != NULL;
		 entry = entry->next)
	{
		size_t key_pos = hashmap_pos_by_capacity(map, entry->key, new_capacity);
		struct hashmap_entry *current_entry = new_buffer[key_pos];

		if (current_entry == NULL)
		{
			new_buffer[key_pos] = entry;
			return;
		}

		for (; current_entry != NULL; current_entry = current_entry->next)
			;
		current_entry->next = entry;
	}

	map->buffer = new_buffer;
	map->capacity = new_capacity;
}

void hashmap_check_size_up(hashmap_t *map)
{
	if (map->size / map->capacity > hashmap_SIZE_UP_RATIO)
		hashmap_rehash(map, map->capacity * 2);
}

void hashmap_check_size_down(hashmap_t *map)
{
	if (map->capacity >= 10 && map->size / map->capacity < hashmap_SIZE_DOWN_RATIO)
		hashmap_rehash(map, map->capacity / 2);
}

hashmap_t *_hashmap_new_(hashmap_config_t config)
{
	size_t capacity = config.capacity > 0
						  ? config.capacity
						  : hashmap_DEFAULT_CAP;

	struct hashmap_entry **buffer = calloc(capacity, sizeof(struct hashmap_entry *));

	return $new(
		hashmap_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_HASHMAP,
#endif
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
		.key_eq_fn = config.key_equal_fn,
#endif
#if hashmap_ALLOW_HASH_FN_OVERLOAD
		.hash_fn = config.hash_fn,
#endif
		.capacity = hashmap_DEFAULT_CAP,
		.size = 0,
		.first_entry = NULL,
		.last_entry = NULL,
		.buffer = buffer);
}

#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
equal_fn_t hashmap_get_key_eq_fn(hashmap_t *map)
{
	return map->key_eq_fn;
}
#endif // hashmap_ALLOW_KEY_EQ_FN_OVERLOAD

#if hashmap_ALLOW_HASH_FN_OVERLOAD
hash_fn_t hashmap_get_hash_fn(hashmap_t *map)
{
	return map->hash_fn;
}
#endif // hashmap_ALLOW_HASH_FN_OVERLOAD

bool hashmap_is_empty(hashmap_t *map)
{
	return map->size == 0;
}

bool hashmap_contains_key(hashmap_t *map, void *key)
{
	size_t key_pos = hashmap_pos(map, key);

	for (struct hashmap_entry *entry = map->buffer[key_pos];
		 entry != NULL;
		 entry = entry->next)
	{
		if (hashmap_keys_eq(map, entry->key, key))
			return true;
	}

	return false;
}

bool hashmap_contains_value(hashmap_t *map, void *value)
{
	for (struct hashmap_entry *node = map->first_entry; node != NULL; node = node->next_entry)
	{
		if (node->value == value)
			return true;
	}

	return false;
}

size_t hashmap_size(hashmap_t *map)
{
	return map->size;
}

void *hashmap_get_at(hashmap_t *map, void *key)
{
	size_t key_pos = hashmap_pos(map, key);

	for (struct hashmap_entry *entry = map->buffer[key_pos];
		 entry != NULL;
		 entry = entry->next)
	{
		if (hashmap_keys_eq(map, entry->key, key))
			return entry->value;
	}

	return NULL;
}

void hashmap_set_at(hashmap_t *map, void *key, void *value)
{
	size_t key_pos = hashmap_pos(map, key);
	struct hashmap_entry *entry = map->buffer[key_pos];

	/* If the table entry doesn't exist, the key isn't in the map. 
	 * Create a new node and put it in the table. */
	if (entry == NULL)
	{
		struct hashmap_entry *new_entry = $new(
			struct hashmap_entry,
			.key = key,
			.value = value,
			.next = NULL,
			.prev_entry = map->last_entry,
			.next_entry = NULL);

		map->buffer[key_pos] = new_entry;

		if (map->first_entry == NULL)
			map->first_entry = map->last_entry = new_entry;
		else
		{
			map->last_entry->next_entry = new_entry;
			map->last_entry = new_entry;
		}

		map->size++;
		return;
	}

	struct hashmap_entry *prev_entry = NULL;

	for (; entry != NULL; prev_entry = entry, entry = entry->next)
	{
		/* If the key is already present in the map, overwrite it. */
		if (hashmap_keys_eq(map, entry->key, key))
		{
			entry->value = value;
			return;
		}
	}

	/* Take a step back in the traversal. */
	entry = prev_entry;

	/* If the key doesn't exist in the map, but its entry is populated, 
	 * create a new node and add it to the list of other keys at that 
	 * entry. */
	struct hashmap_entry *new_entry = $new(
		struct hashmap_entry,
		.key = key,
		.value = value,
		.next = NULL,
		.prev_entry = map->last_entry,
		.next_entry = NULL);

	entry->next = new_entry;

	if (map->first_entry == NULL)
		map->first_entry = map->last_entry = new_entry;
	else
	{
		map->last_entry->next_entry = new_entry;
		map->last_entry = new_entry;
	}

	map->size++;

	hashmap_check_size_up(map);
}

void *hashmap_remove_at(hashmap_t *map, void *key)
{
	size_t key_pos = hashmap_pos(map, key);
	struct hashmap_entry *entry = map->buffer[key_pos];

	for (struct hashmap_entry *prev_entry = NULL;
		 entry != NULL;
		 prev_entry = entry, entry = entry->next)
	{
		if (hashmap_keys_eq(map, entry->key, key))
		{
			if (prev_entry != NULL)
				prev_entry->next = entry->next;
			if (entry->prev_entry)
				entry->prev_entry->next_entry = entry->next_entry;
			if (entry->next_entry)
				entry->next_entry->prev_entry = entry->prev_entry;

			void *value = entry->value;

			free(entry);
			map->size--;

			hashmap_check_size_down(map);
			return value;
		}
	}

	return NULL;
}

void _hashmap_set_all_(hashmap_t *map, size_t n, map_entry_t entries[n])
{
	for (int i = 0; i < n; i++)
	{
		map_entry_t entry = entries[i];
		hashmap_set_at(map, entry.key, entry.value);
	}
}

map_entry_t hashmap_find(hashmap_t *map, bipred_fn_t bipred)
{
	for (struct hashmap_entry *entry = map->first_entry;
		 entry != NULL;
		 entry = entry->next_entry)
	{
		if (bipred(entry->key, entry->value))
			return (map_entry_t){
				.key = entry->key,
				.value = entry->value};
	}

	return (map_entry_t){0};
}

hashset_t *hashmap_keys(hashmap_t *map)
{
	hashset_t *keys = hashset_new();

	for (struct hashmap_entry *entry = map->first_entry;
		 entry != NULL;
		 entry = entry->next_entry)
	{
		hashset_add(keys, entry->key);
	}

	return keys;
}

arraylist_t *hashmap_values(hashmap_t *map)
{
	arraylist_t *values = arraylist_new();

	for (struct hashmap_entry *entry = map->first_entry;
		 entry != NULL;
		 entry = entry->next_entry)
	{
		arraylist_add(values, entry->value);
	}

	return values;
}

hashmap_ref_t *hashmap_ref(hashmap_t *map)
{
	if (map->size == 0)
		return NULL;

	return $new(
		hashmap_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_HASHMAP_REF,
#endif
		.map = map,
		.pos = 0,
		.entry = map->first_entry);
}

bool hashmap_equal(hashmap_t *map1, hashmap_t *map2)
{
	if (map1->size != map2->size)
		return false;

	for (struct hashmap_entry *entry = map1->first_entry;
		 entry != NULL;
		 entry = entry->next_entry)
	{
		if (!hashmap_contains_key(map2, entry->key) ||
			hashmap_get_at(map2, entry->key) != entry->value)
		{
			return false;
		}
	}

	return true;
}

void hashmap_free(hashmap_t *map)
{
	for (struct hashmap_entry *entry = map->first_entry;
		 entry != NULL;)
	{
		struct hashmap_entry *next_entry = entry->next_entry;
		free(entry);
		entry = next_entry;
	}

	free(map->buffer);
	free(map);
}

void *hashmap_ref_get_key(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->entry->key;
}

void *hashmap_ref_get_value(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->entry->value;
}

map_entry_t hashmap_ref_get_entry(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return (map_entry_t){
		.key = ref->entry->key,
		.value = ref->entry->value};
}

hashmap_t *hashmap_ref_get_map(hashmap_ref_t *ref)
{
	return ref->map;
}

size_t hashmap_ref_get_pos(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos;
}

bool hashmap_ref_is_valid(hashmap_ref_t *ref)
{
	return ref->pos != INVALID_REF;
}

bool hashmap_ref_has_prev(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->entry->prev_entry != NULL;
}

bool hashmap_ref_has_next(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->entry->next_entry != NULL;
}

map_entry_t hashmap_ref_next(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!hashmap_ref_has_next(ref))
	{
		ref->pos = INVALID_REF;
		return (map_entry_t){0};
	}
	ref->pos++;
	ref->entry = ref->entry->next_entry;
	return (map_entry_t){.key = ref->entry->key, .value = ref->entry->value};
}

map_entry_t hashmap_ref_prev(hashmap_ref_t *ref)
{
	if (!hashmap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!hashmap_ref_has_prev(ref))
	{
		ref->pos = INVALID_REF;
		return (map_entry_t){0};
	}
	ref->pos--;
	ref->entry = ref->entry->prev_entry;
	return (map_entry_t){.key = ref->entry->key, .value = ref->entry->value};
}

void hashmap_ref_free(hashmap_ref_t *ref)
{
	free(ref);
}

/* ------------------------------------------------------------- */
/*                  ---------- treemap ----------                */
/* ------------------------------------------------------------- */

enum rbtree_color
{
	RBTREE_COLOR_RED = true,
	RBTREE_COLOR_BLACK = false
};

struct rbtree_node
{
	enum rbtree_color color;
	void *key;
	void *value;
	struct rbtree_node *parent;
	struct rbtree_node *left;
	struct rbtree_node *right;
};

bool rbtree_is_leaf(struct rbtree_node *node)
{
	return node != NULL && node->left == NULL && node->right == NULL;
}

bool rbtree_is_red(struct rbtree_node *node)
{
	if (node == NULL)
		return false;
	return node->color == RBTREE_COLOR_RED;
}

bool rbtree_is_black(struct rbtree_node *node)
{
	return !rbtree_is_red(node);
}

void rbtree_set_right(struct rbtree_node *parent, struct rbtree_node *right)
{
	parent->right = right;
	right->parent = parent;
}

void rbtree_set_left(struct rbtree_node *parent, struct rbtree_node *left)
{
	parent->left = left;
	left->parent = parent;
}

struct rbtree_node *rbtree_rotate_left(struct rbtree_node *node)
{
	if (node == NULL)
		panic("RB tree node cannot be NULL");
	else if (!rbtree_is_red(node->right))
		panic("RB tree node must be right-leaning");

	struct rbtree_node *right = node->right;
	rbtree_set_right(node, right->left);
	rbtree_set_left(right, node);

	right->color = right->left->color;
	right->left->color = RBTREE_COLOR_RED;

	return right;
}

struct rbtree_node *rbtree_rotate_right(struct rbtree_node *node)
{
	if (node == NULL)
		panic("RB tree node cannot be NULL");
	else if (!rbtree_is_red(node->left))
		panic("RB tree node must be left-leaning");

	struct rbtree_node *left = node->left;
	rbtree_set_left(node, left->right);
	rbtree_set_right(left, node);

	left->color = left->right->color;
	left->right->color = RBTREE_COLOR_RED;

	return left;
}

void rbtree_flip_colors(struct rbtree_node *node)
{
	if (node == NULL || node->left == NULL || node->right == NULL)
		panic("RB tree node or its children cannot be NULL");
	if (!(rbtree_is_red(node) && rbtree_is_black(node->left) && rbtree_is_black(node->right)) &&
		!(rbtree_is_black(node) && rbtree_is_red(node->left) && rbtree_is_red(node->right)))
		panic("RB tree node must be opposite color of children to be flipped");

	node->color = !node->color;
	node->left->color = !node->left->color;
	node->right->color = !node->right->color;
}

struct rbtree_node *rbtree_move_red_left(struct rbtree_node *node)
{
	if (node == NULL)
		panic("RB tree node cannot be NULL");
	if (rbtree_is_black(node) || rbtree_is_red(node->left) || rbtree_is_red(node->left->left))
		panic("RB tree node must be red and along and its left child and grandchild must be black to be moved left");

	rbtree_flip_colors(node);

	if (rbtree_is_red(node->right->left))
	{
		rbtree_set_right(node, rbtree_rotate_right(node->right));
		node = rbtree_rotate_left(node);
		rbtree_flip_colors(node);
	}

	return node;
}

struct rbtree_node *rbtree_move_red_right(struct rbtree_node *node)
{
	if (node == NULL)
		panic("RB tree node cannot be NULL");
	if (rbtree_is_black(node) || rbtree_is_red(node->right) || rbtree_is_red(node->right->right))
		panic("RB tree node must be red and along and its left child and grandchild must be black to be moved right");

	rbtree_flip_colors(node);

	if (rbtree_is_red(node->left->left)) // TODO: is this right?
	{
		node = rbtree_rotate_right(node);
		rbtree_flip_colors(node);
	}

	return node;
}

struct rbtree_node *rbtree_balance(struct rbtree_node *node)
{
	if (node == NULL)
		panic("RB tree node cannot be NULL");

	if (rbtree_is_red(node->right))
		node = rbtree_rotate_left(node);
	if (rbtree_is_red(node->left) && rbtree_is_red(node->left->left))
		node = rbtree_rotate_right(node);
	if (rbtree_is_red(node->left) && rbtree_is_red(node->right))
		rbtree_flip_colors(node);

	return node;
}

struct rbtree_node *rbtree_min_at(struct rbtree_node *node)
{
	if (node->left == NULL)
		return node;
	return rbtree_min_at(node->left);
}

struct rbtree_node *rbtree_remove_min_at(struct rbtree_node *node)
{
	if (node->left == NULL)
		return NULL;

	if (rbtree_is_black(node->left) && rbtree_is_black(node->left->left))
		node = rbtree_move_red_left(node);

	rbtree_set_left(node, rbtree_remove_min_at(node->left));
	return rbtree_balance(node);
}

struct treemap
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
	compare_fn_t key_cmp_fn;
#endif
	size_t size;
	struct rbtree_node *root;
};

struct treemap_ref
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	treemap_t *map;
	size_t pos;
	struct rbtree_node *node;
	linkedlist_t *inorder_history;
};

int treemap_compare_keys(treemap_t *map, void *key1, void *key2)
{
#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
	return map->key_cmp_fn != NULL ? map->key_cmp_fn(key1, key2) : (key1 < key2 ? -1 : key1 > key2);
#else
	return key1 < key2 ? -1 : key1 > key2;
#endif
}

treemap_t *_treemap_new_(treemap_config_t config)
{
	return $new(
		treemap_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_TREEMAP,
#endif
#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
		.key_cmp_fn = config.key_compare_fn,
#endif
		.size = 0,
		.root = NULL);
}

#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
compare_fn_t treemap_get_key_cmp_fn(treemap_t *map)
{
	return map->key_cmp_fn;
}
#endif

bool treemap_is_empty(treemap_t *map)
{
	return map->size == 0;
}

bool treemap_contains_key(treemap_t *map, void *key)
{
	struct rbtree_node *node;
	for (node = map->root; node != NULL;)
	{
		int key_cmp_val = treemap_compare_keys(map, key, node->key);
		if (key_cmp_val < 0)
			node = node->left;
		else if (key_cmp_val > 0)
			node = node->right;
		else
			return true;
	}

	return false;
}

bool treemap_contains_value_at_node(treemap_t *map, struct rbtree_node *node, void *value)
{
	if (node == NULL)
		return false;

	return node->value == value ||
		   treemap_contains_value_at_node(map, node->left, value) ||
		   treemap_contains_value_at_node(map, node->right, value);
}

bool treemap_contains_value(treemap_t *map, void *value)
{
	return treemap_contains_value_at_node(map, map->root, value);
}

size_t treemap_size(treemap_t *map)
{
	return map->size;
}

void *treemap_get_at(treemap_t *map, void *key)
{
	struct rbtree_node *node;
	for (node = map->root; node != NULL;)
	{
		int key_cmp_val = treemap_compare_keys(map, key, node->key);
		if (key_cmp_val < 0)
			node = node->left;
		else if (key_cmp_val > 0)
			node = node->right;
		else
			return node->value;
	}

	return NULL;
}

struct rbtree_node *treemap_set_at_node(treemap_t *map, struct rbtree_node *node, void *key, void *value)
{
	if (node == NULL)
	{
		struct rbtree_node *new_node = $new(
			struct rbtree_node,
			.color = RBTREE_COLOR_RED,
			.key = key,
			.value = value,
			.parent = NULL,
			.left = NULL,
			.right = NULL);

		map->size++;
		return new_node;
	}

	int key_cmp_val = treemap_compare_keys(map, key, node->key);
	if (key_cmp_val < 0)
		rbtree_set_left(node->left, treemap_set_at_node(map, node->left, key, value));
	else if (key_cmp_val > 0)
		rbtree_set_right(node, treemap_set_at_node(map, node->right, key, value));
	else
		node->value = value;

	if (rbtree_is_red(node->right) && rbtree_is_black(node->left))
		node = rbtree_rotate_left(node);
	if (rbtree_is_red(node->left) && rbtree_is_red(node->left->left))
		node = rbtree_rotate_right(node);
	if (rbtree_is_red(node->left) && rbtree_is_red(node->right))
		rbtree_flip_colors(node);

	return node;
}

void treemap_set_at(treemap_t *map, void *key, void *value)
{
	map->root = treemap_set_at_node(map, map->root, key, value);
	map->root->color = RBTREE_COLOR_BLACK;
}

struct treemap_remove_result
{
	void *value;
	struct rbtree_node *root;
} treemap_remove_at_node(treemap_t *map, struct rbtree_node *node, void *key)
{
	void *value;

	if (node == NULL)
		return (struct treemap_remove_result){NULL, node};

	if (treemap_compare_keys(map, key, node->key) < 0)
	{
		if (rbtree_is_black(node->left) && rbtree_is_black(node->left->left))
			node = rbtree_move_red_left(node);

		struct treemap_remove_result result = treemap_remove_at_node(map, node->left, key);
		rbtree_set_left(node, result.root);
		value = result.value;
	}

	else
	{
		if (rbtree_is_red(node->left))
			node = rbtree_rotate_right(node);
		if (treemap_compare_keys(map, key, node->key) == 0 && node->right == NULL)
			return (struct treemap_remove_result){node->value, NULL};
		if (rbtree_is_black(node->right) && rbtree_is_black(node->right->left))
			node = rbtree_move_red_right(node);

		if (treemap_compare_keys(map, key, node->key) == 0)
		{
			value = node->value;
			struct rbtree_node *min_right = rbtree_min_at(node->right);
			node->key = min_right->key;
			node->value = min_right->value;
			rbtree_set_right(node, rbtree_remove_min_at(node->right));
		}

		else
		{
			struct treemap_remove_result result = treemap_remove_at_node(map, node->right, key);
			rbtree_set_right(node, result.root);
			value = result.value;
		}
	}

	return (struct treemap_remove_result){value, rbtree_balance(node)};
}

void *treemap_remove_at(treemap_t *map, void *key)
{
	if (map->root == NULL)
		return NULL;

	if (rbtree_is_black(map->root->left) && rbtree_is_black(map->root->right))
		map->root->color = RBTREE_COLOR_RED;

	struct treemap_remove_result result = treemap_remove_at_node(map, map->root, key);
	map->root = result.root;

	if (result.value != NULL)
		map->size--;

	if (map->root != NULL)
		map->root->color = RBTREE_COLOR_BLACK;

	return result.value;
}

void _treemap_set_all_(treemap_t *map, size_t n, map_entry_t entries[n])
{
	for (int i = 0; i < n; i++)
	{
		map_entry_t entry = entries[n];
		treemap_set_at(map, entry.key, entry.value);
	}
}

hashset_t *treemap_key_at_node(struct rbtree_node *node, hashset_t *keys)
{
	if (node == NULL)
		return keys;

	hashset_add(keys, node->key);
	treemap_key_at_node(node->left, keys);
	treemap_key_at_node(node->right, keys);

	return keys;
}

map_entry_t treemap_find(treemap_t *map, bipred_fn_t bipred)
{
	for (treemap_ref_t *ref = treemap_ref(map);
		 treemap_ref_is_valid(ref);
		 treemap_ref_next(ref))
	{
		if (bipred(ref->node->key, ref->node->value))
			return (map_entry_t){
				.key = ref->node->key,
				.value = ref->node->value};
	}

	return (map_entry_t){0};
}

hashset_t *treemap_keys(treemap_t *map)
{
	hashset_t *keys = hashset_new();
	return treemap_key_at_node(map->root, keys);
}

arraylist_t *treemap_value_at_node(struct rbtree_node *node, arraylist_t *values)
{
	if (node == NULL)
		return values;

	arraylist_add(values, node->value);
	treemap_value_at_node(node->left, values);
	treemap_value_at_node(node->right, values);

	return values;
}

arraylist_t *treemap_values(treemap_t *map)
{
	arraylist_t *values = arraylist_new();
	return treemap_value_at_node(map->root, values);
}

treemap_ref_t *treemap_ref(treemap_t *map)
{
	if (map->size == 0)
		return NULL;

	treemap_ref_t *ref = $new(
		treemap_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_TREEMAP_REF,
#endif
		.map = map,
		.pos = 0,
		.node = map->root,
		.inorder_history = linkedlist_new());

	/* Initialize this reference at the bottom of the map domain poset. */

	for (; ref->node->left != NULL; ref->node = ref->node->left)
		linkedlist_add_back(ref->inorder_history, ref->node);

	return ref;
}

bool treemap_equal(treemap_t *map1, treemap_t *map2)
{
	if (map1->size != map2->size)
		return false;

	for (treemap_ref_t *ref = treemap_ref(map1);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		void *key = treemap_ref_get_key(ref);
		void *value = treemap_ref_get_value(ref);
		if (!treemap_contains_key(map2, key) || treemap_get_at(map2, key) != value)
			return false;
	}

	return true;
}

void treemap_free_at_node(treemap_t *map, struct rbtree_node *node)
{
	if (node == NULL)
		return;

	treemap_free_at_node(map, node->left);
	treemap_free_at_node(map, node->right);
	free(node);
}

void treemap_free(treemap_t *map)
{
	treemap_free_at_node(map, map->root);
	free(map);
}

void *treemap_ref_get_key(treemap_ref_t *ref)
{
	if (!treemap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->node->key;
}

void *treemap_ref_get_value(treemap_ref_t *ref)
{
	if (!treemap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->node->value;
}

map_entry_t treemap_ref_get_entry(treemap_ref_t *ref)
{
	if (!treemap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return (map_entry_t){
		.key = ref->node->key,
		.value = ref->node->value};
}

treemap_t *treemap_ref_get_map(treemap_ref_t *ref)
{
	return ref->map;
}

size_t treemap_ref_get_pos(treemap_ref_t *ref)
{
	if (!treemap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->pos;
}

bool treemap_ref_is_valid(treemap_ref_t *ref)
{
	return ref->pos != INVALID_REF;
}

bool treemap_ref_has_next(treemap_ref_t *ref)
{
	if (!treemap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	return ref->node != NULL || !linkedlist_is_empty(ref->inorder_history);
}

map_entry_t treemap_ref_next(treemap_ref_t *ref)
{
	if (!treemap_ref_is_valid(ref))
		panic("Reference is out of bounds");
	if (!treemap_ref_has_next(ref))
	{
		ref->pos = INVALID_REF;
		return (map_entry_t){0};
	}

	for (ref->node = ref->node->right;
		 ref->node != NULL;
		 ref->node = ref->node->left)
	{
		linkedlist_add_back(ref->inorder_history, ref->node);
	}

	ref->node = linkedlist_remove_back(ref->inorder_history);
	ref->pos++;

	return (map_entry_t){.key = ref->node->key, .value = ref->node->value};
}

void treemap_ref_free(treemap_ref_t *ref)
{
	linkedlist_free(ref->inorder_history);
	free(ref);
}

/* ------------------------------------------------------------- */
/*                    ---------- set ----------                  */
/* ------------------------------------------------------------- */

#if POLYMORPHIC_DS
#define set_METHOD_DISPATCH(return_t, method, ...)                       \
	({                                                                   \
		switch (set->type)                                               \
		{                                                                \
		case DS_TYPE_HASHSET:                                            \
			return (return_t)hashset_##method((hashset_t *)__VA_ARGS__); \
			break;                                                       \
                                                                         \
		case DS_TYPE_TREESET:                                            \
			return (return_t)treeset_##method((treeset_t *)__VA_ARGS__); \
			break;                                                       \
                                                                         \
		default:                                                         \
			panic("Unknown set type tag: %d", set->type);                \
			return (return_t)0;                                          \
		}                                                                \
	})

#define set_ref_METHOD_DISPATCH(return_t, method, ...)                           \
	({                                                                           \
		switch (ref->type)                                                       \
		{                                                                        \
		case DS_TYPE_HASHSET_REF:                                                \
			return (return_t)hashset_ref_##method((hashset_ref_t *)__VA_ARGS__); \
			break;                                                               \
                                                                                 \
		case DS_TYPE_TREESET_REF:                                                \
			return (return_t)treeset_ref_##method((treeset_ref_t *)__VA_ARGS__); \
			break;                                                               \
                                                                                 \
		default:                                                                 \
			panic("Unknown set ref type tag: %d", ref->type);                    \
			return (return_t)0;                                                  \
		}                                                                        \
	})

struct set
{
	ds_type_t type;
};

struct set_ref
{
	ds_type_t type;
};

size_t set_size(set_t *set)
{
	set_METHOD_DISPATCH(size_t, size, set);
}

bool set_is_empty(set_t *set)
{
	set_METHOD_DISPATCH(bool, is_empty, set);
}

bool set_contains(set_t *set, void *item)
{
	set_METHOD_DISPATCH(bool, contains, set, item);
}

void set_add(set_t *set, void *item)
{
	set_METHOD_DISPATCH(void, add, set, item);
}

void _set_add_all_(set_t *set, size_t nitems, void *items[nitems])
{
	switch (set->type)
	{
	case DS_TYPE_HASHSET:
		_hashset_add_all_((hashset_t *)set, nitems, items);
		break;

	case DS_TYPE_TREESET:
		_treeset_add_all_((treeset_t *)set, nitems, items);
		break;

	default:
		panic("Unknown set type tag: %d", set->type);
	}
}

void *set_remove(set_t *set, void *item)
{
	set_METHOD_DISPATCH(void *, remove, set, item);
}

set_t *set_union(set_t *set1, set_t *set2)
{
	if (is_type(set1, HASHSET) && is_type(set2, HASHSET))
		return (set_t *)hashset_union((hashset_t *)set1, (hashset_t *)set2);
	else if (is_type(set1, TREESET) && is_type(set2, TREESET))
		return (set_t *)treeset_union((treeset_t *)set1, (treeset_t *)set2);

	hashset_t *set_union = hashset_new();
	void *item;

	for (set_ref_t *ref = set_ref(set1);
		 set_ref_has_next(ref);
		 item = set_ref_next(ref))
	{
		hashset_add(set_union, item);
	}

	for (set_ref_t *ref = set_ref(set2);
		 set_ref_has_next(ref);
		 item = set_ref_next(ref))
	{
		hashset_add(set_union, item);
	}

	return (set_t *)set_union;
}

set_t *set_intersection(set_t *set1, set_t *set2)
{
	if (is_type(set1, HASHSET) && is_type(set2, HASHSET))
		return (set_t *)hashset_intersection((hashset_t *)set1, (hashset_t *)set2);
	else if (is_type(set1, TREESET) && is_type(set2, TREESET))
		return (set_t *)treeset_intersection((treeset_t *)set1, (treeset_t *)set2);

	hashset_t *set_intersection = hashset_new();
	void *item;
	for (set_ref_t *ref = set_ref(set1);
		 set_ref_has_next(ref);
		 item = set_ref_next(ref))
	{
		if (set_contains(set2, item))
			hashset_add(set_intersection, item);
	}

	return (set_t *)set_intersection;
}

set_t *set_difference(set_t *set1, set_t *set2)
{
	if (is_type(set1, HASHSET) && is_type(set2, HASHSET))
		return (set_t *)hashset_difference((hashset_t *)set1, (hashset_t *)set2);
	else if (is_type(set1, TREESET) && is_type(set2, TREESET))
		return (set_t *)treeset_difference((treeset_t *)set1, (treeset_t *)set2);

	hashset_t *set_difference = hashset_new();
	void *item;

	for (set_ref_t *ref = set_ref(set1);
		 set_ref_has_next(ref);
		 item = set_ref_next(ref))
	{
		if (!set_contains(set2, item))
			hashset_add(set_difference, item);
	}

	return (set_t *)set_difference;
}

bool set_is_subset(set_t *set, set_t *subset)
{
	return set_is_empty(set_difference(subset, set));
}

void *set_find(set_t *set, pred_fn_t pred)
{
	set_METHOD_DISPATCH(void *, find, set, pred);
}

set_t *set_map(set_t *set, map_fn_t fn)
{
	set_METHOD_DISPATCH(set_t *, map, set, fn);
}

set_t *set_filter(set_t *set, pred_fn_t pred)
{
	set_METHOD_DISPATCH(set_t *, filter, set, pred);
}

void *set_reduce(set_t *set, reduce_fn_t fn)
{
	set_METHOD_DISPATCH(void *, reduce, set, fn);
}

set_ref_t *set_ref(set_t *set)
{
	set_METHOD_DISPATCH(set_ref_t *, ref, set);
}

bool set_equal(set_t *set1, set_t *set2)
{
	void *item;

	if (set_size(set1) != set_size(set2))
		return false;

	for (set_ref_t *ref = set_ref(set1);
		 set_ref_has_next(ref);
		 item = set_ref_next(ref))
	{
		if (!set_contains(set2, item))
			return false;
	}

	return true;
}

void set_free(set_t *set)
{
	set_METHOD_DISPATCH(void, free, set);
}

void *set_ref_get_item(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(void *, get_item, ref);
}

set_t *set_ref_get_set(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(set_t *, get_set, ref);
}

size_t set_ref_get_pos(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(size_t, get_pos, ref);
}

bool set_ref_is_valid(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(bool, is_valid, ref);
}

bool set_ref_has_next(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(bool, has_next, ref);
}

void *set_ref_next(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(void *, next, ref);
}

void set_ref_free(set_ref_t *ref)
{
	set_ref_METHOD_DISPATCH(void, free, ref);
}
#endif

/* ------------------------------------------------------------- */
/*                  ---------- hashset ----------                */
/* ------------------------------------------------------------- */

struct hashset
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	hashmap_t *map;
};

struct hashset_ref
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	hashset_t *set;
	hashmap_ref_t *map_ref;
};

hashset_t *_hashset_new_(hashset_config_t config)
{
	hashmap_t *map = hashmap_new(
#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
			.key_equal_fn = config.equal_fn,
#endif
#if hashmap_ALLOW_HASH_FN_OVERLOAD
			.hash_fn = config.hash_fn
#endif
	);

	return $new(
		hashset_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_HASHSET,
#endif
		.map = map);
}

#if hashmap_ALLOW_KEY_EQ_FN_OVERLOAD
equal_fn_t hashset_get_eq_fn(hashset_t *set)
{
	return hashmap_get_key_eq_fn(set->map);
}
#endif // hashmap_ALLOW_KEY_EQ_FN_OVERLOAD

#if hashmap_ALLOW_HASH_FN_OVERLOAD
hash_fn_t hashset_get_hash_fn(hashset_t *set)
{
	return hashmap_get_hash_fn(set->map);
}
#endif // hashmap_ALLOW_HASH_FN_OVERLOAD

size_t hashset_size(hashset_t *set)
{
	return hashmap_size(set->map);
}

bool hashset_is_empty(hashset_t *set)
{
	return hashmap_is_empty(set->map);
}

bool hashset_contains(hashset_t *set, void *item)
{
	return hashmap_contains_key(set->map, item);
}

void hashset_add(hashset_t *set, void *item)
{
	hashmap_set_at(set->map, item, NULL);
}

void _hashset_add_all_(hashset_t *set, size_t nitems, void *items[nitems])
{
	for (int i = 0; i < nitems; i++)
		hashset_add(set, items[i]);
}

void *hashset_remove(hashset_t *set, void *item)
{
	return hashmap_remove_at(set->map, item);
}

hashset_t *hashset_union(hashset_t *set1, hashset_t *set2)
{
	/* Assumes that set1 and set2 have the same equality function
	 * defined over their items. */

	hashset_t *set_union = hashset_new();

	for (hashmap_ref_t *ref = hashmap_ref(set1->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		hashset_add(set_union, hashmap_ref_get_key(ref));
	}

	for (hashmap_ref_t *ref = hashmap_ref(set2->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		hashset_add(set_union, hashmap_ref_get_key(ref));
	}

	return set_union;
}

hashset_t *hashset_intersection(hashset_t *set1, hashset_t *set2)
{
	/* Assumes that set1 and set2 have the same equality function
	 * defined over their items. */

	hashset_t *set_intersection = hashset_new();

	for (hashmap_ref_t *ref = hashmap_ref(set1->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		if (hashset_contains(set2, hashmap_ref_get_key(ref)))
			hashset_add(set_intersection, hashmap_ref_get_key(ref));
	}

	return set_intersection;
}

hashset_t *hashset_difference(hashset_t *set1, hashset_t *set2)
{
	/* Assumes that set1 and set2 have the same equality function
	 * defined over their items. */

	hashset_t *set_difference = hashset_new();

	for (hashmap_ref_t *ref = hashmap_ref(set1->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		if (!hashset_contains(set2, hashmap_ref_get_key(ref)))
			hashset_add(set_difference, hashmap_ref_get_key(ref));
	}

	return set_difference;
}

void *hashset_find(hashset_t *set, pred_fn_t pred)
{
	for (hashset_ref_t *ref = hashset_ref(set);
		 hashset_ref_is_valid(ref);
		 hashset_ref_next(ref))
	{
		if (pred(hashset_ref_get_item(ref)))
			return hashset_ref_get_item(ref);
	}

	return NULL;
}

bool hashset_is_subset(hashset_t *set, hashset_t *subset)
{
	/* Assumes that set and subset have the same equality function
	 * defined over their items. */

	return hashset_is_empty(hashset_difference(subset, set));
}

hashset_t *hashset_map(hashset_t *set, map_fn_t fn)
{
	hashset_t *set_mapped = hashset_new();

	for (hashmap_ref_t *ref = hashmap_ref(set->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		hashset_add(set_mapped, fn(hashmap_ref_get_key(ref)));
	}

	return set_mapped;
}

hashset_t *hashset_filter(hashset_t *set, pred_fn_t pred)
{
	hashset_t *set_filtered = hashset_new();

	for (hashmap_ref_t *ref = hashmap_ref(set->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		if (pred(hashmap_ref_get_key(ref)))
			hashset_add(set_filtered, hashmap_ref_get_key(ref));
	}

	return set_filtered;
}

void *hashset_reduce(hashset_t *set, reduce_fn_t fn)
{
	void **work_buffer = calloc(set->map->size, sizeof(void *));

	int i = 0;
	for (hashmap_ref_t *ref = hashmap_ref(set->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref), i++)
	{
		work_buffer[i] = hashmap_ref_get_key(ref);
	}

	for (int stride = 1; stride < set->map->size; stride *= 2)
	{
		for (int i = 0; i < set->map->size; i += (stride * 2))
		{
			if (i + stride < set->map->size)
				work_buffer[i] = fn(work_buffer[i], work_buffer[i + stride]);
		}
	}

	void *result = work_buffer[0];
	free(work_buffer);
	return result;
}

hashset_ref_t *hashset_ref(hashset_t *set)
{
	hashset_ref_t *ref = $new(
		hashset_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_HASHSET_REF,
#endif
		.set = set,
		.map_ref = hashmap_ref(set->map));

	return ref;
}

bool hashset_equal(hashset_t *set1, hashset_t *set2)
{
	if (set1->map->size != set2->map->size)
		return false;

	for (hashmap_ref_t *ref = hashmap_ref(set1->map);
		 hashmap_ref_has_next(ref);
		 hashmap_ref_next(ref))
	{
		if (!hashset_contains(set2, hashmap_ref_get_key(ref)))
			return false;
	}

	return true;
}

void hashset_free(hashset_t *set)
{
	hashmap_free(set->map);
	free(set);
}

void *hashset_ref_get_item(hashset_ref_t *ref)
{
	return hashmap_ref_get_key(ref->map_ref);
}

hashset_t *hashset_ref_get_set(hashset_ref_t *ref)
{
	return ref->set;
}

size_t hashset_ref_get_pos(hashset_ref_t *ref)
{
	return hashmap_ref_get_pos(ref->map_ref);
}

bool hashset_ref_is_valid(hashset_ref_t *ref)
{
	return hashmap_ref_is_valid(ref->map_ref);
}

bool hashset_ref_has_prev(hashset_ref_t *ref)
{
	return hashmap_ref_has_prev(ref->map_ref);
}

bool hashset_ref_has_next(hashset_ref_t *ref)
{
	return hashmap_ref_has_next(ref->map_ref);
}

void *hashset_ref_next(hashset_ref_t *ref)
{
	return hashmap_ref_next(ref->map_ref).key;
}

void *hashset_ref_prev(hashset_ref_t *ref)
{
	return hashmap_ref_prev(ref->map_ref).key;
}

void hashset_ref_free(hashset_ref_t *ref)
{
	hashmap_ref_free(ref->map_ref);
	free(ref);
}

/* ------------------------------------------------------------- *
 *                  ---------- treeset ----------                *
 * ------------------------------------------------------------- */

struct treeset
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	treemap_t *map;
};

struct treeset_ref
{
#if POLYMORPHIC_DS
	ds_type_t type;
#endif
	treeset_t *set;
	treemap_ref_t *map_ref;
};

treeset_t *_treeset_new_(treeset_config_t config)
{
	treemap_t *map = treemap_new(
#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
			.key_compare_fn = config.compare_fn
#endif
	);

	return $new(
		treeset_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_TREESET,
#endif
		.map = map);
}

#if treemap_ALLOW_KEY_CMP_FN_OVERLOAD
compare_fn_t treeset_get_cmp_fn(treeset_t *set)
{
	return treemap_get_key_cmp_fn(set->map);
}
#endif // treemap_ALLOW_KEY_CMP_FN_OVERLOAD

size_t treeset_size(treeset_t *set)
{
	return treemap_size(set->map);
}

bool treeset_is_empty(treeset_t *set)
{
	return treemap_is_empty(set->map);
}

bool treeset_contains(treeset_t *set, void *item)
{
	return treemap_contains_key(set->map, item);
}

void treeset_add(treeset_t *set, void *item)
{
	treemap_set_at(set->map, item, NULL);
}

void _treeset_add_all_(treeset_t *set, size_t nitems, void *items[nitems])
{
	for (int i = 0; i < nitems; i++)
		treeset_add(set, items[i]);
}

void *treeset_remove(treeset_t *set, void *item)
{
	return treemap_remove_at(set->map, item);
}

treeset_t *treeset_union(treeset_t *set1, treeset_t *set2)
{
	/* Assumes that set1 and set2 have the same comparison function
	 * defined over their items. */

	treeset_t *set_union = treeset_new();

	for (treemap_ref_t *ref = treemap_ref(set1->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		treeset_add(set_union, treemap_ref_get_key(ref));
	}

	for (treemap_ref_t *ref = treemap_ref(set2->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		treeset_add(set_union, treemap_ref_get_key(ref));
	}

	return set_union;
}

treeset_t *treeset_intersection(treeset_t *set1, treeset_t *set2)
{
	/* Assumes that set1 and set2 have the same comparison function
	 * defined over their items. */

	treeset_t *set_intersection = treeset_new();

	for (treemap_ref_t *ref = treemap_ref(set1->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		if (treeset_contains(set2, treemap_ref_get_value(ref)))
			treeset_add(set_intersection, treemap_ref_get_key(ref));
	}

	return set_intersection;
}

treeset_t *treeset_difference(treeset_t *set1, treeset_t *set2)
{
	/* Assumes that set1 and set2 have the same comparison function
	 * defined over their items. */

	treeset_t *set_intersection = treeset_new();

	for (treemap_ref_t *ref = treemap_ref(set1->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		if (!treeset_contains(set2, treemap_ref_get_value(ref)))
			treeset_add(set_intersection, treemap_ref_get_key(ref));
	}

	return set_intersection;
}

bool treeset_is_subset(treeset_t *set, treeset_t *subset)
{
	return treeset_is_empty(treeset_difference(subset, set));
}

void *treeset_find(treeset_t *set, pred_fn_t pred)
{
	for (treeset_ref_t *ref = treeset_ref(set);
		 treeset_ref_is_valid(ref);
		 treeset_ref_next(ref))
	{
		if (pred(treeset_ref_get_item(ref)))
			return treeset_ref_get_item(ref);
	}

	return NULL;
}

treeset_t *treeset_map(treeset_t *set, map_fn_t fn)
{
	treeset_t *set_mapped = treeset_new();

	for (treemap_ref_t *ref = treemap_ref(set->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		treeset_add(set_mapped, fn(treemap_ref_get_key(ref)));
	}

	return set_mapped;
}

treeset_t *treeset_filter(treeset_t *set, pred_fn_t pred)
{
	treeset_t *set_filtered = treeset_new();

	for (treemap_ref_t *ref = treemap_ref(set->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		if (pred(treemap_ref_get_key(ref)))
			treeset_add(set_filtered, treemap_ref_get_key(ref));
	}

	return set_filtered;
}

void *treeset_reduce(treeset_t *set, reduce_fn_t fn)
{
	void **work_buffer = calloc(set->map->size, sizeof(void *));

	int i = 0;
	for (treemap_ref_t *ref = treemap_ref(set->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref), i++)
	{
		work_buffer[i] = treemap_ref_get_key(ref);
	}

	for (int stride = 1; stride < set->map->size; stride *= 2)
	{
		for (int i = 0; i < set->map->size; i += (stride * 2))
		{
			if (i + stride < set->map->size)
				work_buffer[i] = fn(work_buffer[i], work_buffer[i + stride]);
		}
	}

	void *result = work_buffer[0];
	free(work_buffer);
	return result;
}

treeset_ref_t *treeset_ref(treeset_t *set)
{
	return $new(
		treeset_ref_t,
#if POLYMORPHIC_DS
		.type = DS_TYPE_TREESET_REF,
#endif
		.set = set,
		.map_ref = treemap_ref(set->map));
}

bool treeset_equal(treeset_t *set1, treeset_t *set2)
{
	if (set1->map->size != set2->map->size)
		return false;

	for (treemap_ref_t *ref = treemap_ref(set1->map);
		 treemap_ref_has_next(ref);
		 treemap_ref_next(ref))
	{
		if (!treeset_contains(set2, treemap_ref_get_key(ref)))
			return false;
	}

	return true;
}

void treeset_free(treeset_t *set)
{
	treemap_free(set->map);
	free(set);
}

void *treeset_ref_get_item(treeset_ref_t *ref)
{
	return treemap_ref_get_key(ref->map_ref);
}

treeset_t *treeset_ref_get_set(treeset_ref_t *ref)
{
	return ref->set;
}

size_t treeset_ref_get_pos(treeset_ref_t *ref)
{
	return treemap_ref_get_pos(ref->map_ref);
}

bool treeset_ref_is_valid(treeset_ref_t *ref)
{
	return treemap_ref_is_valid(ref->map_ref);
}

bool treeset_ref_has_next(treeset_ref_t *ref)
{
	return treemap_ref_has_next(ref->map_ref);
}

void *treeset_ref_next(treeset_ref_t *ref)
{
	return treemap_ref_next(ref->map_ref).key;
}

void treeset_ref_free(treeset_ref_t *ref)
{
	treemap_ref_free(ref->map_ref);
	free(ref);
}