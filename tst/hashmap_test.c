#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/functions.h"
#include "../src/debug.h"
#include "../src/testing.h"

hashmap_t *map;

DEFAULT_RESOURCE_HANDLER_ALL

void before_each()
{
#if SHOULD_MEMORY_DEBUG
	debug_mem_setup();
#endif
	map = hashmap_new();
}

void after_each()
{
	hashmap_free(map);
}

void test_new()
{
	assert_equal(0, hashmap_size(map));
}

void test_is_empty()
{
	assert_true(hashmap_is_empty(map));
	hashmap_set_at(map, "0", _(0));
	assert_false(hashmap_is_empty(map));
	hashmap_remove_at(map, "0");
	assert_true(hashmap_is_empty(map));
}

void test_contains_key()
{
	hashmap_set_at(map, "0", _(0));
	assert_true(hashmap_contains_key(map, "0"));
	assert_false(hashmap_contains_key(map, "1"));
	hashmap_set_at(map, "1", _(1));
	assert_true(hashmap_contains_key(map, "1"));
}

void test_contains_value()
{
	hashmap_set_at(map, "0", _(0));
	assert_true(hashmap_contains_value(map, _(0)));
	assert_false(hashmap_contains_value(map, _(1)));
	hashmap_set_at(map, "1", _(1));
	assert_true(hashmap_contains_value(map, _(1)));
}

void test_size()
{
	assert_equal(0, hashmap_size(map));

	for (int i = 0; i < 5; i++)
	{
		hashmap_set_at(map, _(i), _(i));
		assert_equal(i + 1, hashmap_size(map));
	}

	for (int i = 4; i >= 0; i--)
	{
		hashmap_remove_at(map, _(i));
		assert_equal(i, hashmap_size(map));
	}
}

void test_get_at()
{
	hashmap_set_all(
		map,
		{_(0), _(0)},
		{_(1), _(-1)},
		{_(2), _(-2)});

	for (int i = 0; i < 3; i++)
		assert_equal(-i, (long)hashmap_get_at(map, _(i)));

	for (int i = 0; i < 3; i++)
	{
		hashmap_remove_at(map, _(i));
		assert_equal(hashmap_get_at(map, _(i)), NULL);
	}
}

void test_set_at()
{
	hashmap_set_at(map, _(5), _(-5));
	assert_equal(-5, (long)hashmap_get_at(map, _(5)));
	hashmap_set_at(map, _(5), _(-10));
	assert_equal(-10, (long)hashmap_get_at(map, _(5)));
	hashmap_set_at(map, _(10), _(5));
	assert_equal(5, (long)hashmap_get_at(map, _(10)));
}

void test_remove_at()
{
	hashmap_set_all(
		map,
		{_(0), _(0)},
		{_(1), _(-1)},
		{_(2), _(-2)});

	for (int i = 0; i < 3; i++)
	{
		assert_equal(-i, (long)hashmap_remove_at(map, _(i)));
		assert_equal(hashmap_get_at(map, _(i)), NULL);
	}

	for (int i = 0; i < 3; i++)
		assert_equal(hashmap_remove_at(map, _(i)), NULL);
}

bool sum_greater_than_10(const void *first, const void *second)
{
	return (long)first + (long)second > 10;
}

bool both_less_than_0(const void *first, const void *second)
{
	return first < 0 && second < 0;
}

void test_find()
{
	for (int i = 0; i < 10; i++)
		hashmap_set_at(map, _(i), _(i));

	map_entry_t entry = hashmap_find(map, sum_greater_than_10);
	assert_equal(entry.key, _(6));
	assert_equal(entry.value, _(6));

	entry = hashmap_find(map, both_less_than_0);
	assert_equal(entry.key, NULL);
	assert_equal(entry.value, NULL);
}

void test_keys()
{
	hashmap_set_all(map, {_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	hashset_t *keys = hashmap_keys(map);
	assert_true(hashset_contains(keys, _(0)));
	assert_true(hashset_contains(keys, _(1)));
	assert_true(hashset_contains(keys, _(2)));
	assert_false(hashset_contains(keys, _(3)));
}

void test_values()
{
	hashmap_set_all(map, {_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	arraylist_t *values = hashmap_values(map);
	assert_true(arraylist_contains(values, _(0)));
	assert_true(arraylist_contains(values, _(-1)));
	assert_true(arraylist_contains(values, _(-2)));
	assert_false(arraylist_contains(values, _(-3)));
}

void test_equal()
{
	hashmap_set_all(map, {_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	hashmap_t *map2 = hashmap({_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	hashmap_t *map3 = hashmap({_(0), _(1)}, {_(1), _(-1)}, {_(2), _(-2)});
	hashmap_t *map4 = hashmap({_(0), _(0)}, {_(1), _(-1)});
	assert_true(hashmap_equal(map, map2));
	assert_false(hashmap_equal(map, map3));
	assert_false(hashmap_equal(map, map4));
}

void test_hash_fn_key_eq_fn()
{
	hashmap_free(map);
	map = hashmap_new(.key_equal_fn = streq, .hash_fn = strhash);
	hashmap_set_all(map, {"first", _(1)}, {"second", _(2)}, {"third", _(3)});

	assert_true(hashmap_contains_key(map, "first"));
	assert_true(hashmap_contains_key(map, "second"));
	assert_true(hashmap_contains_key(map, "third"));

	hashmap_set_at(map, "second", _(-2));
	assert_equal(hashmap_get_at(map, "second"), _(-2));
}

void test_ref_forward_iter()
{
	for (int i = 0; i < 10; i++)
		hashmap_set_at(map, _(i), _(-i));

	hashmap_ref_t *ref = hashmap_ref(map);

	for (int i = 0; i < 10; i++)
	{
		assert_true(hashmap_ref_is_valid(ref));

		assert_equal(hashmap_ref_get_key(ref), _(i));
		assert_equal(hashmap_ref_get_value(ref), _(-i));

		assert_struct_equal(((map_entry_t){_(i), _(-i)}),
							hashmap_ref_get_entry(ref));

		if (i < 9)
			assert_true(hashmap_ref_has_next(ref));

		hashmap_ref_next(ref);
	}
}

void test_ref_backward_iter()
{
	for (int i = 0; i < 10; i++)
		hashmap_set_at(map, _(i), _(-i));

	hashmap_ref_t *ref = hashmap_ref(map);

	for (int i = 0; i < 9; i++)
		hashmap_ref_next(ref);

	for (int i = 9; i >= 0; i--)
	{
		assert_true(hashmap_ref_is_valid(ref));

		assert_equal(_(i), hashmap_ref_get_key(ref));
		assert_equal(_(-i), hashmap_ref_get_value(ref));

		assert_struct_equal(((map_entry_t){_(i), _(-i)}),
							hashmap_ref_get_entry(ref));

		if (i > 0)
			assert_true(hashmap_ref_has_prev(ref));

		hashmap_ref_prev(ref);
	}
}

int main(int argc, char *argv[])
{
	TEST_SUITE(
		TEST(test_new),
		TEST(test_is_empty),
		TEST(test_contains_key),
		TEST(test_contains_value),
		TEST(test_size),
		TEST(test_get_at),
		TEST(test_set_at),
		TEST(test_remove_at),
		TEST(test_find),
		TEST(test_keys),
		TEST(test_values),
		TEST(test_equal),
		TEST(test_hash_fn_key_eq_fn),
		TEST(test_ref_forward_iter),
		TEST(test_ref_backward_iter));
}