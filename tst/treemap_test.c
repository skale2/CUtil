#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/functions.h"
#include "../src/debug.h"
#include "../src/testing.h"

treemap_t *map;

DEFAULT_RESOURCE_HANDLER_ALL

void before_each()
{
#if SHOULD_MEMORY_DEBUG
	debug_mem_setup();
#endif
	map = treemap_new();
}

void after_each()
{
	treemap_free(map);
}

void test_new()
{
	assert_equal(0, treemap_size(map));
}

void test_is_empty()
{
	assert_true(treemap_is_empty(map));
	treemap_set_at(map, "0", _(0));
	assert_false(treemap_is_empty(map));
	treemap_remove_at(map, "0");
	assert_true(treemap_is_empty(map));
}

void test_contains_key()
{
	treemap_set_at(map, "0", _(0));
	assert_true(treemap_contains_key(map, "0"));
	assert_false(treemap_contains_key(map, "1"));
	treemap_set_at(map, "1", _(1));
	assert_true(treemap_contains_key(map, "1"));
}

void test_contains_value()
{
	treemap_set_at(map, "0", _(0));
	assert_true(treemap_contains_value(map, _(0)));
	assert_false(treemap_contains_value(map, _(1)));
	treemap_set_at(map, "1", _(1));
	assert_true(treemap_contains_value(map, _(1)));
}

void test_size()
{
	assert_equal(0, treemap_size(map));

	for (int i = 0; i < 5; i++)
	{
		treemap_set_at(map, _(i), _(i));
		assert_equal(i + 1, treemap_size(map));
	}

	for (int i = 4; i >= 0; i--)
	{
		treemap_remove_at(map, _(i));
		assert_equal(i, treemap_size(map));
	}
}

void test_get_at()
{
	treemap_set_all(
		map,
		{_(0), _(0)},
		{_(1), _(-1)},
		{_(2), _(-2)});

	for (int i = 0; i < 3; i++)
		assert_equal(-i, (long)treemap_get_at(map, _(i)));

	for (int i = 0; i < 3; i++)
	{
		treemap_remove_at(map, _(i));
		assert_equal(treemap_get_at(map, _(i)), NULL);
	}
}

void test_set_at()
{
	treemap_set_at(map, _(5), _(-5));
	assert_equal(-5, (long)treemap_get_at(map, _(5)));
	treemap_set_at(map, _(5), _(-10));
	assert_equal(-10, (long)treemap_get_at(map, _(5)));
	treemap_set_at(map, _(10), _(5));
	assert_equal(5, (long)treemap_get_at(map, _(10)));
}

void test_remove_at()
{
	treemap_set_all(
		map,
		{_(0), _(0)},
		{_(1), _(-1)},
		{_(2), _(-2)});

	for (int i = 0; i < 3; i++)
	{
		assert_equal(-i, (long)treemap_remove_at(map, _(i)));
		assert_equal(treemap_get_at(map, _(i)), NULL);
	}

	for (int i = 0; i < 3; i++)
		assert_equal(treemap_remove_at(map, _(i)), NULL);
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
		treemap_set_at(map, _(i), _(i));

	map_entry_t entry = treemap_find(map, sum_greater_than_10);
	assert_equal(entry.key, _(6));
	assert_equal(entry.value, _(6));

	entry = treemap_find(map, both_less_than_0);
	assert_equal(entry.key, NULL);
	assert_equal(entry.value, NULL);
}

void test_keys()
{
	treemap_set_all(map, {_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	hashset_t *keys = treemap_keys(map);
	assert_true(hashset_contains(keys, _(0)));
	assert_true(hashset_contains(keys, _(1)));
	assert_true(hashset_contains(keys, _(2)));
	assert_false(hashset_contains(keys, _(3)));
}

void test_values()
{
	treemap_set_all(map, {_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	arraylist_t *values = treemap_values(map);
	assert_true(arraylist_contains(values, _(0)));
	assert_true(arraylist_contains(values, _(-1)));
	assert_true(arraylist_contains(values, _(-2)));
	assert_false(arraylist_contains(values, _(-3)));
}

void test_equal()
{
	treemap_set_all(map, {_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	treemap_t *map2 = treemap({_(0), _(0)}, {_(1), _(-1)}, {_(2), _(-2)});
	treemap_t *map3 = treemap({_(0), _(1)}, {_(1), _(-1)}, {_(2), _(-2)});
	treemap_t *map4 = treemap({_(0), _(0)}, {_(1), _(-1)});
	assert_true(treemap_equal(map, map2));
	assert_false(treemap_equal(map, map3));
	assert_false(treemap_equal(map, map4));
}

void test_sort_cmp_fn()
{
	treemap_free(map);
	map = treemap_new(.key_compare_fn = (compare_fn_t)strcmp);
	treemap_set_all(map, {"tree", _(0)}, {"capital", _(1)}, {"CAPITAL", _(2)}, {"something", _(3)});

	treemap_ref_t *ref = treemap_ref(map);

	assert_equal((char *)"CAPITAL", treemap_ref_get_key(ref));
	assert_equal(_(2), treemap_ref_get_value(ref));
	assert_true(treemap_ref_is_valid(ref));

	treemap_ref_next(ref);

	assert_equal((char *)"capital", treemap_ref_get_key(ref));
	assert_equal(_(1), treemap_ref_get_value(ref));
	assert_true(treemap_ref_is_valid(ref));

	treemap_ref_next(ref);

	assert_equal((char *)"something", treemap_ref_get_key(ref));
	assert_equal(_(3), treemap_ref_get_value(ref));
	assert_true(treemap_ref_is_valid(ref));

	treemap_ref_next(ref);

	assert_equal((char *)"tree", treemap_ref_get_key(ref));
	assert_equal(_(0), treemap_ref_get_value(ref));
	assert_true(treemap_ref_is_valid(ref));

	treemap_ref_next(ref);
	assert_false(treemap_ref_is_valid(ref));
}

void test_ref_forward_iter()
{
	for (int i = 0; i < 10; i++)
		treemap_set_at(map, _(i), _(-i));

	treemap_ref_t *ref = treemap_ref(map);

	for (int i = 0; i < 10; i++)
	{
		assert_true(treemap_ref_is_valid(ref));

		assert_equal(treemap_ref_get_key(ref), _(i));
		assert_equal(treemap_ref_get_value(ref), _(-i));

		assert_struct_equal(((map_entry_t){_(i), _(-i)}),
							treemap_ref_get_entry(ref));

		if (i < 9)
			assert_true(treemap_ref_has_next(ref));

		treemap_ref_next(ref);
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
		TEST(test_iter_cmp_fn),
		TEST(test_ref_forward_iter));
}