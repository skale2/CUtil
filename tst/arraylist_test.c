#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/functions.h"
#include "../src/debug.h"
#include "../src/testing.h"

arraylist_t *list;

DEFAULT_RESOURCE_HANDLER_ALL

void before_each()
{
#if SHOULD_MEMORY_DEBUG
	debug_mem_setup();
#endif
	list = arraylist_new();
}

void after_each()
{
	arraylist_free(list);
}

void test_new()
{
	assert_equal(0, arraylist_size(list));
}

void test_add()
{
	arraylist_add(list, _(-1));
	arraylist_add(list, _(-2));
	assert_equal( 2, arraylist_size(list));
	assert_equal(-1, (long)arraylist_get_at(list, 0));
	assert_equal(-2, (long)arraylist_get_at(list, 1));
}

void test_resize_up()
{
	for (size_t i = 0; i < 10; i++)
		arraylist_add(list, _(i));

	assert_equal(10, arraylist_size(list));

	for (size_t i = 0; i < 10; i++)
		assert_equal(i, (long)arraylist_get_at(list, i));
}

void test_is_empty()
{
	assert_true(arraylist_is_empty(list));
	arraylist_add(list, _(1));
	assert_false(arraylist_is_empty(list));
}

void test_get_at()
{
	assert_panic(arraylist_get_at(list, 0));

	arraylist_add(list, _(0));
	arraylist_add(list, _(1));
	arraylist_add(list, _(2));

	assert_equal(0, (long)arraylist_get_at(list, 0));
	assert_equal(1, (long)arraylist_get_at(list, 1));
	assert_equal(2, (long)arraylist_get_at(list, 2));
	assert_panic(arraylist_get_at(list, 3));

	assert_equal(0, (long)arraylist_get_at(list, -3));
	assert_equal(1, (long)arraylist_get_at(list, -2));
	assert_equal(2, (long)arraylist_get_at(list, -1));
	assert_panic(arraylist_get_at(list, -4));
}

void test_get_first()
{
	assert_panic(arraylist_get_first(list));
	arraylist_add(list, _(1));
	assert_equal(1, (long)arraylist_get_first(list));
	arraylist_add_back(list, _(2));
	assert_equal(1, (long)arraylist_get_first(list));
	arraylist_add_front(list, _(0));
	assert_equal(0, (long)arraylist_get_first(list));
}

void test_get_last()
{
	assert_panic(arraylist_get_last(list));
	arraylist_add(list, _(-1));
	assert_equal(-1, (long)arraylist_get_last(list));
	arraylist_add_front(list, _(-2));
	assert_equal(-1, (long)arraylist_get_last(list));
	arraylist_add_back(list, _(0));
	assert_equal( 0, (long)arraylist_get_last(list));
}

void test_set_at()
{
	arraylist_add(list, _(1));
	arraylist_add(list, _(2));
	arraylist_add(list, _(3));
	arraylist_add(list, _(4));
	arraylist_add(list, _(5));

	arraylist_set_at(list, 1, _(-2));
	assert_equal(-2, (long)arraylist_get_at(list, 1));

	arraylist_set_at(list, -2, _(-4));
	assert_equal(-4, (long)arraylist_get_at(list, 3));

	arraylist_set_at(list, -4, _(12));
	assert_equal(12, (long)arraylist_get_at(list, 1));
}

void test_add_at()
{
	arraylist_add(list, _(0));
	arraylist_add(list, _(2));

	arraylist_add_at(list, 1, _(1));

	assert_equal(0, (long)arraylist_get_at(list, 0));
	assert_equal(1, (long)arraylist_get_at(list, 1));
	assert_equal(2, (long)arraylist_get_at(list, 2));

	assert_panic(arraylist_add_at(list, 3, _(3)));
}

void test_add_all()
{
	arraylist_add_all(list, _(0), _(1), _(2));
	assert_equal(arraylist_size(list), 3);
	assert_equal(0, (long)arraylist_get_at(list, 0));
	assert_equal(1, (long)arraylist_get_at(list, 1));
	assert_equal(2, (long)arraylist_get_at(list, 2));
}

void test_literal()
{
	arraylist_free(list);
	list = arraylist(_(0), _(1), _(2));
	assert_equal(3, arraylist_size(list));
	assert_equal(0, (long)arraylist_get_at(list, 0));
	assert_equal(1, (long)arraylist_get_at(list, 1));
	assert_equal(2, (long)arraylist_get_at(list, 2));
}

void test_pos_of()
{
	arraylist_add_all(list, _(2), _(15), _(-1));
	assert_equal(0, arraylist_pos_of(list, (void *)2));
	assert_equal( 1, arraylist_pos_of(list, (void *)15));
	assert_equal( 2, arraylist_pos_of(list, (void *)-1));
	assert_equal(-1, arraylist_pos_of(list, (void *)1));
}

void test_remove()
{
	arraylist_add_all(list, _(0), _(1), _(2));

	assert_equal(1, (long)arraylist_remove(list, (void *)1));
	assert_equal(0, (long)arraylist_get_at(list, 0));
	assert_equal(2, (long)arraylist_get_at(list, 1));

	assert_equal((long)NULL, (long)arraylist_remove(list, (void *)-4));
}

void test_remove_front()
{
	arraylist_add_all(list, _(0), _(1), _(2));

	assert_equal(0, (long)arraylist_remove_front(list));
	assert_equal(1, (long)arraylist_remove_front(list));
	assert_equal(2, (long)arraylist_remove_front(list));
	assert_equal((long)NULL, (long)arraylist_remove_front(list));

	assert_equal(0, arraylist_size(list));
}

void test_remove_back()
{
	arraylist_add_all(list, _(0), _(1), _(2));

	assert_equal(2, (long)arraylist_remove_back(list));
	assert_equal(1, (long)arraylist_remove_back(list));
	assert_equal(0, (long)arraylist_remove_back(list));
	assert_equal((long)NULL, (long)arraylist_remove_back(list));

	assert_equal(0, arraylist_size(list));
}

void test_remove_at()
{
	arraylist_add_all(list, _(0), _(1), _(2));

	assert_equal(0, (long)arraylist_remove_at(list, 0));
	assert_equal(2, (long)arraylist_remove_at(list, 1));
	assert_equal(1, (long)arraylist_remove_at(list, 0));

	assert_equal(0, arraylist_size(list));
	assert_panic(arraylist_remove_at(list, 0));
}

void test_resize_down()
{
	arraylist_free(list);

	list = arraylist_new(.capacity = 30);
	for (int i = 0; i < 20; i++)
		arraylist_add(list, _(i));

	for (int i = 19; i >= 0; i--)
		assert_equal(i, (long)arraylist_remove_back(list));

	assert_equal(0, arraylist_size(list));
}

void test_resize_up_and_down()
{
	arraylist_free(list);

	list = arraylist_new(.capacity = 5);
	for (int i = 0; i < 20; i++)
		arraylist_add(list, _(i));

	for (int i = 19; i >= 0; i--)
		assert_equal(i, (long)arraylist_remove_back(list));

	assert_equal(0, arraylist_size(list));
}

void test_concat()
{
	arraylist_t *first = arraylist(_(1), _(2), _(3));
	arraylist_t *second = arraylist(_(4), _(5), _(6));

	arraylist_t *concat = arraylist_concat(first, second);

	for (int i = 0; i < 6; i++)
		assert_equal(_(i + 1), arraylist_get_at(concat, i));
}

void *add_one(const void *x)
{
	return _(((long)x) + 1);
}

void test_map()
{
	arraylist_add_all(list, _(0), _(1), _(2));
	arraylist_t *new_list = arraylist_map(list, add_one);

	assert_equal(3, arraylist_size(new_list));

	for (int i = 0; i < 3; i++)
		assert_equal(i + 1, (long)arraylist_get_at(new_list, i));
}

bool only_positive(const void *x)
{
	return (long)x > 0;
}

void test_filter()
{
	arraylist_add_all(list, _(-2), _(-1), _(0), _(1), _(2));
	arraylist_t *new_list = arraylist_filter(list, only_positive);

	assert_equal(2, arraylist_size(new_list));

	for (int i = 1; i <= 2; i++)
		assert_equal(i, (long)arraylist_get_at(new_list, i - 1));
}

void *sum(void *a, void *b)
{
	return (void *)((long)a + (long)b);
}

void test_reduce()
{
	free(list);
	list = arraylist(_(0), _(1), _(2), _(3), _(4), _(5));
	assert_equal(15, (long)arraylist_reduce(list, sum));
	list = arraylist(_(0), _(1), _(2), _(3), _(4), _(5), _(6));
	assert_equal(21, (long)arraylist_reduce(list, sum));
}

void test_equal()
{
	free(list);
	list = arraylist(_(0), _(1), _(2));
	arraylist_t *list2 = arraylist(_(0), _(1), _(2));
	arraylist_t *list3 = arraylist(_(0), _(1), _(2), _(3));
	arraylist_t *list4 = arraylist(_(1), _(-2), _(-5));

	assert_true(arraylist_equal(list, list2));
	assert_false(arraylist_equal(list, list3));
	assert_false(arraylist_equal(list, list4));
}

bool _strcmp(const char *a, const char *b)
{
	return strcmp(a, b) == 0;
}

void test_equal_item_eq_fn()
{
	arraylist_add_all(list, "string1", "string2", "string3");
	arraylist_t *list2 = arraylist("string1", "string2", "string3");
	arraylist_t *list3 = arraylist("string1", "string2", "not string 3");

	assert_true(arraylist_equal(list, list2, .eq_fn = (equal_fn_t)_strcmp));
	assert_false(arraylist_equal(list, list3, .eq_fn = (equal_fn_t)_strcmp));
}

void test_ref_forward_iter()
{
	arraylist_add_all(list, _(0), _(1), _(2));
	arraylist_ref_t *ref = arraylist_ref(list);

	assert_equal(0, (long)arraylist_ref_get_item(ref));
	assert_true(arraylist_ref_has_next(ref));
	arraylist_ref_next(ref);

	assert_equal(1, (long)arraylist_ref_get_item(ref));
	assert_true(arraylist_ref_has_next(ref));
	arraylist_ref_next(ref);

	assert_equal(2, (long)arraylist_ref_get_item(ref));
	assert_false(arraylist_ref_has_next(ref));

	arraylist_ref_free(ref);
}

void test_ref_for_loop()
{
	arraylist_add_all(list, _(0), _(1), _(2));

	for (arraylist_ref_t *ref = arraylist_ref(list);
		 arraylist_ref_is_valid(ref);
		 arraylist_ref_next(ref))
	{
		void *item = arraylist_ref_get_item(ref);
		assert_equal((long) item, arraylist_ref_get_pos(ref));
	}
}

void test_ref_backwards_iter()
{
	arraylist_add_all(list, _(0), _(1), _(2));
	arraylist_ref_t *ref = arraylist_ref(list);

	for (int i = 0; i < 2; i++)
		arraylist_ref_next(ref);

	assert_equal(2, (long)arraylist_ref_get_item(ref));
	assert_true(arraylist_ref_has_prev(ref));
	arraylist_ref_prev(ref);

	assert_equal(1, (long)arraylist_ref_get_item(ref));
	assert_true(arraylist_ref_has_prev(ref));
	arraylist_ref_prev(ref);

	assert_equal(0, (long)arraylist_ref_get_item(ref));
	assert_false(arraylist_ref_has_prev(ref));

	arraylist_ref_free(ref);
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
		TEST(test_ref_for_loop),
		TEST(test_ref_forward_iter),
		TEST(test_ref_backwards_iter));
}