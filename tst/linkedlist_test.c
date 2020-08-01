#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/functions.h"
#include "../src/debug.h"
#include "../src/testing.h"

linkedlist_t *list;

DEFAULT_RESOURCE_HANDLER_ALL

void before_each()
{
#if SHOULD_MEMORY_DEBUG
	debug_mem_setup();
#endif
	list = linkedlist_new();
}

void after_each()
{
	linkedlist_free(list);
}

void test_new()
{
	assert_equal(0, linkedlist_size(list));
}

void test_add()
{
	linkedlist_add(list, _(-1));
	linkedlist_add(list, _(-2));
	assert_equal(2, linkedlist_size(list));
	assert_equal(-1, (long)linkedlist_get_at(list, 0));
	assert_equal(-2, (long)linkedlist_get_at(list, 1));
}

void test_resize_up()
{
	for (size_t i = 0; i < 10; i++)
		linkedlist_add(list, _(i));

	assert_equal(10, linkedlist_size(list));

	for (size_t i = 0; i < 10; i++)
		assert_equal(i, (long)linkedlist_get_at(list, i));
}

void test_is_empty()
{
	assert_true(linkedlist_is_empty(list));
	linkedlist_add(list, _(1));
	assert_false(linkedlist_is_empty(list));
}

void test_get_at()
{
	assert_panic(linkedlist_get_at(list, 0));

	linkedlist_add(list, _(0));
	linkedlist_add(list, _(1));
	linkedlist_add(list, _(2));

	assert_equal(0, (long)linkedlist_get_at(list, 0));
	assert_equal(1, (long)linkedlist_get_at(list, 1));
	assert_equal(2, (long)linkedlist_get_at(list, 2));
	assert_panic(linkedlist_get_at(list, 3));

	assert_equal(0, (long)linkedlist_get_at(list, -3));
	assert_equal(1, (long)linkedlist_get_at(list, -2));
	assert_equal(2, (long)linkedlist_get_at(list, -1));
	assert_panic(linkedlist_get_at(list, -4));
}

void test_get_first()
{
	assert_panic(linkedlist_get_first(list));
	linkedlist_add(list, _(1));
	assert_equal(1, (long)linkedlist_get_first(list));
	linkedlist_add_back(list, _(2));
	assert_equal(1, (long)linkedlist_get_first(list));
	linkedlist_add_front(list, _(0));
	assert_equal(0, (long)linkedlist_get_first(list));
}

void test_get_last()
{
	assert_panic(linkedlist_get_last(list));
	linkedlist_add(list, _(-1));
	assert_equal(-1, (long)linkedlist_get_last(list));
	linkedlist_add_front(list, _(-2));
	assert_equal(-1, (long)linkedlist_get_last(list));
	linkedlist_add_back(list, _(0));
	assert_equal(0, (long)linkedlist_get_last(list));
}

void test_set_at()
{
	linkedlist_add(list, _(1));
	linkedlist_add(list, _(2));
	linkedlist_add(list, _(3));
	linkedlist_add(list, _(4));
	linkedlist_add(list, _(5));

	linkedlist_set_at(list, 1, _(-2));
	assert_equal(-2, (long)linkedlist_get_at(list, 1));

	linkedlist_set_at(list, -2, _(-4));
	assert_equal(-4, (long)linkedlist_get_at(list, 3));

	linkedlist_set_at(list, -4, _(12));
	assert_equal(12, (long)linkedlist_get_at(list, 1));
}

void test_add_at()
{
	linkedlist_add(list, _(0));
	linkedlist_add(list, _(2));

	linkedlist_add_at(list, 1, _(1));

	assert_equal(0, (long)linkedlist_get_at(list, 0));
	assert_equal(1, (long)linkedlist_get_at(list, 1));
	assert_equal(2, (long)linkedlist_get_at(list, 2));

	assert_panic(linkedlist_add_at(list, 3, _(3)));
}

void test_add_all()
{
	linkedlist_add_all(list, _(0), _(1), _(2));
	assert_equal(3, linkedlist_size(list));
	assert_equal(0, (long)linkedlist_get_at(list, 0));
	assert_equal(1, (long)linkedlist_get_at(list, 1));
	assert_equal(2, (long)linkedlist_get_at(list, 2));
}

void test_literal()
{
	linkedlist_free(list);
	list = linkedlist(_(0), _(1), _(2));
	assert_equal(3, linkedlist_size(list));
	assert_equal(0, (long)linkedlist_get_at(list, 0));
	assert_equal(1, (long)linkedlist_get_at(list, 1));
	assert_equal(2, (long)linkedlist_get_at(list, 2));
}

void test_pos_of()
{
	linkedlist_add_all(list, _(2), _(15), _(-1));
	assert_equal(0, linkedlist_pos_of(list, _(2)));
	assert_equal(1, linkedlist_pos_of(list, _(15)));
	assert_equal(2, linkedlist_pos_of(list, _(-1)));
	assert_equal(-1, linkedlist_pos_of(list, _(1)));
}

void test_remove()
{
	linkedlist_add_all(list, _(0), _(1), _(2));

	assert_equal(1, (long)linkedlist_remove(list, _(1)));
	assert_equal(0, (long)linkedlist_get_at(list, 0));
	assert_equal(2, (long)linkedlist_get_at(list, 1));
	assert_equal(2, linkedlist_size(list));

	assert_equal((long)NULL, (long)linkedlist_remove(list, _(-4)));
}

void test_remove_front()
{
	linkedlist_add_all(list, _(0), _(1), _(2));

	assert_equal(0, (long)linkedlist_remove_front(list));
	assert_equal(1, (long)linkedlist_remove_front(list));
	assert_equal(2, (long)linkedlist_remove_front(list));
	assert_equal((long)NULL, (long)linkedlist_remove_front(list));

	assert_equal(0, linkedlist_size(list));
}

void test_remove_back()
{
	linkedlist_add_all(list, _(0), _(1), _(2));

	assert_equal(2, (long)linkedlist_remove_back(list));
	assert_equal(1, (long)linkedlist_remove_back(list));
	assert_equal(0, (long)linkedlist_remove_back(list));
	assert_equal((long)NULL, (long)linkedlist_remove_back(list));

	assert_equal(0, linkedlist_size(list));
}

void test_remove_at()
{
	linkedlist_add_all(list, _(0), _(1), _(2));

	assert_equal(0, (long)linkedlist_remove_at(list, 0));
	assert_equal(2, (long)linkedlist_remove_at(list, 1));
	assert_equal(1, (long)linkedlist_remove_at(list, 0));

	assert_equal(0, linkedlist_size(list));
	assert_panic(linkedlist_remove_at(list, 0));
}

void test_resize_down()
{
	linkedlist_free(list);

	list = linkedlist();
	for (int i = 0; i < 20; i++)
		linkedlist_add(list, _(i));

	for (int i = 19; i >= 0; i--)
		assert_equal(i, (long)linkedlist_remove_back(list));

	assert_equal(0, linkedlist_size(list));
}

void test_resize_up_and_down()
{
	linkedlist_free(list);

	list = linkedlist();
	for (int i = 0; i < 20; i++)
		linkedlist_add(list, _(i));

	for (int i = 19; i >= 0; i--)
		assert_equal(i, (long)linkedlist_remove_back(list));

	assert_equal(0, linkedlist_size(list));
}

void test_concat()
{
	linkedlist_t *first = linkedlist(_(1), _(2), _(3));
	linkedlist_t *second = linkedlist(_(4), _(5), _(6));

	linkedlist_t *concat = linkedlist_concat(first, second);

	for (int i = 0; i < 6; i++)
		assert_equal(linkedlist_get_at(concat, i), _(i + 1));
}

void *add_one(const void *x)
{
	return _(((long)x) + 1);
}

void test_map()
{
	linkedlist_add_all(list, _(0), _(1), _(2));
	linkedlist_t *new_list = linkedlist_map(list, add_one);

	assert_equal(3, linkedlist_size(new_list));
	for (int i = 0; i < 3; i++)
		assert_equal(i + 1, (long)linkedlist_get_at(new_list, i));
}

bool only_positive(const void *x)
{
	return (long)x > 0;
}

void test_filter()
{
	linkedlist_add_all(list, _(-2), _(-1), _(0), _(1), _(2));
	linkedlist_t *new_list = linkedlist_filter(list, only_positive);

	assert_equal(2, linkedlist_size(new_list));
	for (int i = 1; i <= 2; i++)
		assert_equal(i, (long)linkedlist_get_at(new_list, i - 1));
}

void *sum(const void *a, const void *b)
{
	return (void *)((long)a + (long)b);
}

void test_reduce()
{
	linkedlist_free(list);
	list = linkedlist(_(0), _(1), _(2), _(3), _(4), _(5));
	assert_equal(15, (long)linkedlist_reduce(list, sum));
	list = linkedlist(_(0), _(1), _(2), _(3), _(4), _(5), _(6));
	assert_equal(21, (long)linkedlist_reduce(list, sum));
}

void test_equal()
{
	free(list);
	list = linkedlist(_(0), _(1), _(2));
	linkedlist_t *list2 = linkedlist(_(0), _(1), _(2));
	linkedlist_t *list3 = linkedlist(_(0), _(1), _(2), _(3));
	linkedlist_t *list4 = linkedlist(_(1), _(-2), _(-5));

	assert_true(linkedlist_equal(list, list2));
	assert_false(linkedlist_equal(list, list3));
	assert_false(linkedlist_equal(list, list4));
}

bool _strcmp(char *a, char *b)
{
	return strcmp(a, b) == 0;
}

void test_equal_item_eq_fn()
{
	free(list);
	list = linkedlist("string1", "string2", "string3");
	linkedlist_t *list2 = linkedlist("string1", "string2", "string3");
	linkedlist_t *list3 = linkedlist("string1", "string2", "not string 3");

	assert_true(linkedlist_equal_item_eq_fn(list, list2, (equal_fn_t)_strcmp));
	assert_false(linkedlist_equal_item_eq_fn(list, list3, (equal_fn_t)_strcmp));
}

void test_ref_forward_iter()
{
	linkedlist_add_all(list, _(0), _(1), _(2));
	linkedlist_ref_t *ref = linkedlist_ref(list);

	assert_equal(0, (long)linkedlist_ref_get_item(ref));
	assert_true(linkedlist_ref_has_next(ref));
	linkedlist_ref_next(ref);

	assert_equal(1, (long)linkedlist_ref_get_item(ref));
	assert_true(linkedlist_ref_has_next(ref));
	linkedlist_ref_next(ref);

	assert_equal(2, (long)linkedlist_ref_get_item(ref));
	assert_false(linkedlist_ref_has_next(ref));

	linkedlist_ref_free(ref);
}

void test_ref_backwards_iter()
{
	linkedlist_add_all(list, _(0), _(1), _(2));
	linkedlist_ref_t *ref = linkedlist_ref(list);

	for (int i = 0; i < 2; i++)
		linkedlist_ref_next(ref);

	assert_equal(2, (long)linkedlist_ref_get_item(ref));
	assert_true(linkedlist_ref_has_prev(ref));
	linkedlist_ref_prev(ref);

	assert_equal(1, (long)linkedlist_ref_get_item(ref));
	assert_true(linkedlist_ref_has_prev(ref));
	linkedlist_ref_prev(ref);

	assert_equal(0, (long)linkedlist_ref_get_item(ref));
	assert_false(linkedlist_ref_has_prev(ref));

	linkedlist_ref_free(ref);
}

int main(int argc, char *argv[])
{
	TEST_SUITE(
		TEST(test_new),
		TEST(test_add),
		TEST(test_resize_up),
		TEST(test_is_empty),
		TEST(test_get_at),
		TEST(test_get_first),
		TEST(test_get_last),
		TEST(test_set_at),
		TEST(test_add_at),
		TEST(test_add_all),
		TEST(test_literal),
		TEST(test_pos_of),
		TEST(test_remove),
		TEST(test_remove_front),
		TEST(test_remove_back),
		TEST(test_remove_at),
		TEST(test_resize_down),
		TEST(test_resize_up_and_down),
		TEST(test_concat),
		TEST(test_map),
		TEST(test_filter),
		TEST(test_reduce),
		TEST(test_equal),
		TEST(test_equal_item_eq_fn),
		TEST(test_ref_forward_iter),
		TEST(test_ref_backwards_iter));
}