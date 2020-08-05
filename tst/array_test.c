#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/debug.h"
#include "../src/testing.h"

struct foo
{
    int a, b, c;
    float z;
};

array_t(int) arr;
array_t(struct foo) arr2;

testing_DEFAULT_RESOURCE_HANDLER_ALL

void before_each()
{
#if SHOULD_MEMORY_DEBUG
    debug_mem_setup();
#endif
    arr = NULL;
    arr2 = NULL;
}

void after_each()
{
    array_free(arr);
    array_free(arr2);
}

void test_new()
{
    arr = array_new(int, 3);
    array_at(arr, 0) = 0;
    array_at(arr, 1) = -1;
    array_at(arr, 2) = -2;

    assert_equal(0, array_at(arr, 0));
    assert_equal(-1, array_at(arr, 1));
    assert_equal(-2, array_at(arr, 2));

    arr2 = array_new(struct foo, 3);
    array_at(arr2, 0) = (struct foo){0, 0, 0, 0.0};
    array_at(arr2, 1) = (struct foo){-1, -1, -1, -1.0};
    array_at(arr2, 2) = (struct foo){-2, -2, -2, -2.0};

    struct foo expected, given;

    assert_struct_equal(((struct foo){0, 0, 0, 0.0}), array_at(arr2, 0));
    assert_struct_equal(((struct foo){-1, -1, -1, -1.0}), array_at(arr2, 1));
    assert_struct_equal(((struct foo){-2, -2, -2, -2.0}), array_at(arr2, 2));
}

void test_literal()
{
    arr = array(int, 0, -1, -2);

    assert_equal(0, array_at(arr, 0));
    assert_equal(-1, array_at(arr, 1));
    assert_equal(-2, array_at(arr, 2));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});

    struct foo expected, given;

    assert_struct_equal(((struct foo){0, 0, 0, 0.0}), array_at(arr2, 0));
    assert_struct_equal(((struct foo){-1, -1, -1, -1.0}), array_at(arr2, 1));
    assert_struct_equal(((struct foo){-2, -2, -2, -2.0}), array_at(arr2, 2));
}

void test_size()
{
    arr = array(int);
    assert_equal(0, array_size(arr));
    array_free(arr);

    arr = array(int, 0, -1, -2);
    assert_equal(3, array_size(arr));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0});

    assert_equal(2, array_size(arr2));
}

void test_is_empty()
{
    arr = array(int);
    assert_true(array_is_empty(arr));
    array_free(arr);

    arr = array(int, 0, -1, -2);
    assert_false(array_is_empty(arr));

    arr2 = array(struct foo);
    assert_true(array_is_empty(arr2));
    array_free(arr2);

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0});
    assert_false(array_is_empty(arr2));
}

void test_get_first()
{
    arr = array(int);
    assert_panic(array_get_first(arr));
    array_free(arr);

    arr = array(int, 0, -1, -2);
    assert_equal(0, array_get_first(arr));

    arr2 = array(struct foo);
    assert_panic(array_get_first(arr2));
    array_free(arr2);

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});

    assert_struct_equal(((struct foo){0, 0, 0, 0.0}),
                        array_get_first(arr2));
}

void test_get_last()
{
    arr = array(int);
    assert_panic(array_get_last(arr));
    array_free(arr);

    arr = array(int, 0, -1, -2);
    assert_equal(-2, array_get_last(arr));

    arr2 = array(struct foo);
    assert_panic(array_get_last(arr2));
    array_free(arr2);

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});

    assert_struct_equal(((struct foo){-2, -2, -2, -2.0}),
                        array_get_last(arr2));
}

void test_concat()
{
    arr = array(int, 0, -1, -2);
    array_t(int) arr_s = array(int, -3, -4, -5);

    array_t(int) arr_f = array_concat(arr, arr_s);
    assert_equal(6, array_size(arr_f));

    for (int i = 0; i < 6; i++)
        assert_equal(-i, arr_f[i]);

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});

    array_t(struct foo) arr2_s = array(struct foo,
                                       (struct foo){-3, -3, -3, -3.0},
                                       (struct foo){-4, -4, -4, -4.0},
                                       (struct foo){-5, -5, -5, -5.0});

    array_t(struct foo) arr2_f = array_concat(arr2, arr2_s);
    assert_equal(6, array_size(arr2_f));

    for (int i = 0; i < 6; i++)
        assert_struct_equal(((struct foo){-i, -i, -i, (float)(-i)}),
                            arr2_f[i]);

    array_free(arr_s);
    array_free(arr_f);
    array_free(arr2_s);
    array_free(arr2_f);
}

void test_pos_of()
{
    arr = array(int, 0, -1, -2);

    assert_equal(0, array_pos_of(arr, 0));
    assert_equal(1, array_pos_of(arr, -1));
    assert_equal(2, array_pos_of(arr, -2));
    assert_equal(-1, array_pos_of(arr, -3));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});

    assert_equal(0, array_pos_of(arr2, (struct foo){0 cm 0 cm 0 cm 0.0}));
    assert_equal(1, array_pos_of(arr2, (struct foo){-1 cm - 1 cm - 1 cm - 1.0}));
    assert_equal(2, array_pos_of(arr2, (struct foo){-2 cm - 2 cm - 2 cm - 2.0}));
    assert_equal(-1, array_pos_of(arr2, (struct foo){-3 cm - 3 cm - 3 cm - 3.0}));
}

bool int_less_than_neg_5_ref(const void *ref)
{
    return *(int *)ref < -5;
}

bool foo_a_less_than_neg_5_ref(const void *ref)
{
    return ((struct foo *)ref)->a < -5;
}

void test_find()
{
    arr = array_new(int, 10);
    for (int i = 0; i < 10; i++)
        array_at(arr, i) = -i;

    assert_equal(-6, array_find(arr, int_less_than_neg_5_ref));

    arr2 = array_new(struct foo, 10);
    for (int i = 0; i < 10; i++)
        array_at(arr2, i) = (struct foo){.a = -i};

    assert_struct_equal((struct foo){.a = -6},
                        array_find(arr2, foo_a_less_than_neg_5_ref));
}

void *double_ref(const void *item)
{
    return $box(*(int *)item * 2);
}

void *double_a_ref(const void *item)
{
    struct foo *foo = (struct foo *)item;
    return $new(struct foo, foo->a * 2, foo->b, foo->c, foo->z);
}

void test_map()
{
    arr = array(int, 0, -1, -2);
    array_t(int) new_arr = array_map(arr, double_ref);

    assert_equal(0, array_at(new_arr, 0));
    assert_equal(-2, array_at(new_arr, 1));
    assert_equal(-4, array_at(new_arr, 2));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});
    array_t(struct foo) new_arr2 = array_map(arr2, double_a_ref);

    assert_struct_equal(((struct foo){0, 0, 0, 0.0}), array_at(new_arr2, 0));
    assert_struct_equal(((struct foo){-2, -1, -1, -1.0}), array_at(new_arr2, 1));
    assert_struct_equal(((struct foo){-4, -2, -2, -2.0}), array_at(new_arr2, 2));
}

void test_filter()
{
    arr = array_new(int, 10);
    for (int i = 0; i < 10; i++)
        array_at(arr, i) = -i;

    array_t(int) new_arr = array_filter(arr, int_less_than_neg_5_ref);

    assert_equal(4, array_size(new_arr));

    for (int i = 6; i < 10; i++)
        assert_equal(-i, array_at(new_arr, i - 6));

    arr2 = array_new(struct foo, 10);
    for (int i = 0; i < 10; i++)
        array_at(arr2, i) = (struct foo){.a = -i};

    array_t(struct foo) new_arr2 = array_filter(arr2, foo_a_less_than_neg_5_ref);

    for (int i = 6; i < 10; i++)
        assert_equal(-i, array_at(new_arr2, i - 6).a);
}

void *sum(const void *a, const void *b)
{
    return $box(*(int *)a + *(int *)b);
}

void *foo_sum(const void *a, const void *b)
{
    struct foo *fa = a;
    struct foo *fb = b;
    return $new(struct foo,
                fa->a + fb->a,
                fa->b + fb->b,
                fa->c + fb->c,
                fa->z + fb->z);
}

void test_reduce()
{
    arr = array(int, -5, 4, -10, 1, 6, -2);
    assert_equal(-6, array_reduce(arr, sum));

    arr = array(int, -5, 4, -10, 6, 1);
    assert_equal(-4, array_reduce(arr, sum));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});

    assert_struct_equal(((struct foo){-3, -3, -3, -3.0}),
                        array_reduce(arr2, foo_sum));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0},
                 (struct foo){-3, -3, -3, -3.0});

    assert_struct_equal(((struct foo){-6, -6, -6, -6.0}),
                        array_reduce(arr2, foo_sum));
}

void test_slice()
{
    arr = array(int, 1, 2, 3);
    array_t(int *) slc = array_slice(arr);

    assert_equal(3, array_size(slc));

    assert_equal(1, *array_at(slc, 0));
    assert_equal(2, *array_at(slc, 1));
    assert_equal(3, *array_at(slc, 2));

    arr2 = array(struct foo,
                 (struct foo){0, 0, 0, 0.0},
                 (struct foo){-1, -1, -1, -1.0},
                 (struct foo){-2, -2, -2, -2.0});
    array_t(struct foo *) slc2 = array_slice(arr2);

    assert_equal(3, array_size(slc));

    assert_struct_equal(array_at(arr2, 0), *array_at(slc2, 0));
    assert_struct_equal(array_at(arr2, 1), *array_at(slc2, 1));
    assert_struct_equal(array_at(arr2, 2), *array_at(slc2, 2));
}

void test_range()
{
    arr = range(0, 10);
    assert_equal(10, array_size(arr));

    for (int i = 0; i < array_size(arr); i++)
        assert_equal(i, array_at(arr, i));

    array_t(char) arrc = range((char)'a', (char)'z' + 1);
    assert_equal('z' - 'a' + 1, array_size(arrc));

    for (char c = 'a'; c <= 'z'; c++)
        assert_equal(c, array_at(arrc, c - 'a'));

    array_free(arrc);

    array_t(int64_t) arrl = range_step((int64_t)-1, (int64_t)-5, -1);
    assert_equal(4, array_size(arrl));

    for (int i = 0; i < array_size(arrl); i++)
        assert_equal((int64_t)(-1 - i), array_at(arrl, i));

    array_t(unsigned short) arrs = range_step((unsigned short)20, (unsigned short)11, -2);
    assert_equal(5, array_size(arrs));

    for (int i = 0; i < array_size(arrs); i++)
        assert_equal((unsigned short)(-2*i + 20), array_at(arrs, i));
}                                                                        

int main(int argc, char *argv[])
{
    TEST_SUITE(
        TEST(test_new),
        TEST(test_literal),
        TEST(test_size),
        TEST(test_is_empty),
        TEST(test_get_first),
        TEST(test_get_last),
        TEST(test_concat),
        TEST(test_pos_of),
        TEST(test_find),
        TEST(test_map),
        TEST(test_filter),
        TEST(test_reduce),
        TEST(test_slice),
        TEST(test_range));
}