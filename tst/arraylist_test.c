#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/debug.h"
#include "../src/testing.h"

#define _(o) ((void *)(long)(o))

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

void test_arraylist_new()
{
	assert_uint_equal(arraylist_size(list), 0);
}

void test_arraylist_add()
{
	arraylist_add(list, (void *)-1);
	arraylist_add(list, (void *)-2);

	assert_uint_equal(arraylist_size(list), 2);
	assert_int_equal((long)arraylist_get_at(list, 0), -1);
	assert_int_equal((long)arraylist_get_at(list, 1), -2);
}

void test_arraylist_resize_up()
{
	for (size_t i = 0; i < 10; i++)
		arraylist_add(list, (void *)i);

	assert_uint_equal(arraylist_size(list), 10);

	for (size_t i = 0; i < 10; i++)
		assert_int_equal((long)arraylist_get_at(list, i), i);
}

void test_arraylist_is_empty()
{
	assert_true(arraylist_is_empty(list));
	arraylist_add(list, (void *)1);
	assert_false(arraylist_is_empty(list));
}

void test_arraylist_get_at()
{
	assert_panic(arraylist_get_at(list, 0));

	arraylist_add(list, (void *)0);
	arraylist_add(list, (void *)1);
	arraylist_add(list, (void *)2);

	assert_int_equal((long)arraylist_get_at(list, 0), 0);
	assert_int_equal((long)arraylist_get_at(list, 1), 1);
	assert_int_equal((long)arraylist_get_at(list, 2), 2);
	assert_panic(arraylist_get_at(list, 3));

	assert_int_equal((long)arraylist_get_at(list, -3), 0);
	assert_int_equal((long)arraylist_get_at(list, -2), 1);
	assert_int_equal((long)arraylist_get_at(list, -1), 2);
	assert_panic(arraylist_get_at(list, -4));
}

void test_arraylist_get_first()
{
	assert_panic(arraylist_get_first(list));
	arraylist_add(list, (void *)1);
	assert_int_equal((long)arraylist_get_first(list), 1);
	arraylist_add_back(list, (void *)2);
	assert_int_equal((long)arraylist_get_first(list), 1);
	arraylist_add_front(list, (void *)0);
	assert_int_equal((long)arraylist_get_first(list), 0);
}

void test_arraylist_get_last()
{
	assert_panic(arraylist_get_last(list));
	arraylist_add(list, (void *)-1);
	assert_int_equal((long)arraylist_get_last(list), -1);
	arraylist_add_front(list, (void *)-2);
	assert_int_equal((long)arraylist_get_last(list), -1);
	arraylist_add_back(list, (void *)0);
	assert_int_equal((long)arraylist_get_last(list), 0);
}

void test_arraylist_set_at()
{
	arraylist_add(list, (void *)1);
	arraylist_add(list, (void *)2);
	arraylist_add(list, (void *)3);
	arraylist_add(list, (void *)4);
	arraylist_add(list, (void *)5);

	arraylist_set_at(list, 1, (void *)-2);
	assert_int_equal((long)arraylist_get_at(list, 1), -2);

	arraylist_set_at(list, -2, (void *)-4);
	assert_int_equal((long)arraylist_get_at(list, 3), -4);

	arraylist_set_at(list, -4, (void *)12);
	assert_int_equal((long)arraylist_get_at(list, 1), 12);
}

void test_arraylist_add_at()
{
	arraylist_add(list, (void *)0);
	arraylist_add(list, (void *)2);

	arraylist_add_at(list, 1, (void *)1);

	assert_int_equal((long)arraylist_get_at(list, 0), 0);
	assert_int_equal((long)arraylist_get_at(list, 1), 1);
	assert_int_equal((long)arraylist_get_at(list, 2), 2);

	assert_panic(arraylist_add_at(list, 3, (void *)3));
}

void test_arraylist_add_all()
{
	arraylist_add_all(list, 3, (void *[]){(void *)0, (void *)1, (void *)2});
	assert_uint_equal(arraylist_size(list), 3);
	assert_int_equal((long)arraylist_get_at(list, 0), 0);
	assert_int_equal((long)arraylist_get_at(list, 1), 1);
	assert_int_equal((long)arraylist_get_at(list, 2), 2);
}

void test_arraylist_literal()
{
	arraylist_free(list);
	list = arraylist((void *)0, (void *)1, (void *)2);
	assert_uint_equal(arraylist_size(list), 3);
	assert_int_equal((long)arraylist_get_at(list, 0), 0);
	assert_int_equal((long)arraylist_get_at(list, 1), 1);
	assert_int_equal((long)arraylist_get_at(list, 2), 2);
}

void test_arraylist_pos_of()
{
	arraylist_add_all(list, 3, (void *[]){(void *)2, (void *)15, (void *)-1});
	assert_uint_equal(arraylist_pos_of(list, (void *)2), 0);
	assert_uint_equal(arraylist_pos_of(list, (void *)15), 1);
	assert_uint_equal(arraylist_pos_of(list, (void *)-1), 2);
	assert_uint_equal(arraylist_pos_of(list, (void *)1), -1);
}

void test_arraylist_remove()
{
	arraylist_add_all(list, 3, (void *[]){(void *)0, (void *)1, (void *)2});

	assert_int_equal((long)arraylist_remove(list, (void *)1), 1);
	assert_int_equal((long)arraylist_get_at(list, 0), 0);
	assert_int_equal((long)arraylist_get_at(list, 1), 2);

	assert_int_equal((long)arraylist_remove(list, (void *)-4), (long)NULL);
}

void test_arraylist_remove_front()
{
	arraylist_add_all(list, 3, (void *[]){(void *)0, (void *)1, (void *)2});

	assert_int_equal((long)arraylist_remove_front(list), 0);
	assert_int_equal((long)arraylist_remove_front(list), 1);
	assert_int_equal((long)arraylist_remove_front(list), 2);
	assert_int_equal((long)arraylist_remove_front(list), (long)NULL);

	assert_uint_equal(arraylist_size(list), 0);
}

void test_arraylist_remove_back()
{
	arraylist_add_all(list, 3, (void *[]){(void *)0, (void *)1, (void *)2});

	assert_int_equal((long)arraylist_remove_back(list), 2);
	assert_int_equal((long)arraylist_remove_back(list), 1);
	assert_int_equal((long)arraylist_remove_back(list), 0);
	assert_int_equal((long)arraylist_remove_back(list), (long)NULL);

	assert_uint_equal(arraylist_size(list), 0);
}

void test_arraylist_remove_at()
{
	arraylist_add_all(list, 3, (void *[]){(void *)0, (void *)1, (void *)2});

	assert_int_equal((long)arraylist_remove_at(list, 0), 0);
	assert_int_equal((long)arraylist_remove_at(list, 1), 2);
	assert_int_equal((long)arraylist_remove_at(list, 0), 1);

	assert_uint_equal(arraylist_size(list), 0);
}

void test_arraylist_resize_down()
{
	arraylist_free(list);

	list = arraylist_new_cap(30);
	for (int i = 0; i < 20; i++)
		arraylist_add(list, _(i));

	for (int i = 19; i >= 0; i--)
		assert_int_equal((long)arraylist_remove_back(list), i);

	assert_uint_equal(arraylist_size(list), 0);
}

void test_arraylist_resize_up_and_down()
{
	arraylist_free(list);

	list = arraylist_new_cap(5);
	for (int i = 0; i < 20; i++)
		arraylist_add(list, _(i));

	for (int i = 19; i >= 0; i--)
		assert_int_equal((long)arraylist_remove_back(list), i);

	assert_uint_equal(arraylist_size(list), 0);
}

void *add_one(void *x)
{
	return (void *)(((long)x) + 1);
}

void test_arraylist_map()
{
	arraylist_add_all(list, 3, (void *[]){_(0), _(1), _(2)});
	arraylist_t *new_list = arraylist_map(list, add_one);

	assert_uint_equal(arraylist_size(new_list), 3);
	for (int i = 0; i < 3; i++)
		assert_int_equal((long)arraylist_get_at(new_list, i), i + 1);
}

bool only_positive(void *x)
{
	return (long)x > 0;
}

void test_arraylist_filter()
{
	arraylist_add_all(list, 5, (void *[]){_(-2), _(-1), _(0), _(1), _(2)});
	arraylist_t *new_list = arraylist_filter(list, only_positive);

	assert_uint_equal(arraylist_size(new_list), 2);
	for (int i = 1; i <= 2; i++)
		assert_int_equal((long)arraylist_get_at(new_list, i-1), i);
}

int main(int argc, char *argv[])
{
	TEST_SUITE(
		TEST(test_arraylist_new),
		TEST(test_arraylist_add),
		TEST(test_arraylist_resize_up),
		TEST(test_arraylist_is_empty),
		TEST(test_arraylist_get_at),
		TEST(test_arraylist_get_first),
		TEST(test_arraylist_get_last),
		TEST(test_arraylist_set_at),
		TEST(test_arraylist_add_at),
		TEST(test_arraylist_add_all),
		TEST(test_arraylist_literal),
		TEST(test_arraylist_pos_of),
		TEST(test_arraylist_remove),
		TEST(test_arraylist_remove_front),
		TEST(test_arraylist_remove_back),
		TEST(test_arraylist_remove_at),
		TEST(test_arraylist_resize_down),
		TEST(test_arraylist_resize_up_and_down),
		TEST(test_arraylist_map),
		TEST(test_arraylist_filter));
}