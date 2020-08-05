#include <stdio.h>
#include "../src/data_struct.h"
#include "../src/functions.h"
#include "../src/debug.h"
#include "../src/testing.h"

struct foo
{
    int i;
    float f;
};

linkedlist_t(int) list1 = NULL;
linkedlist_ref_t(int) ref1 = NULL;

linkedlist_t(struct foo) list2 = NULL;
linkedlist_ref_t(struct foo) ref2 = NULL;

testing_DEFAULT_RESOURCE_HANDLER_ALL

    void
    before_each()
{
#if SHOULD_MEMORY_DEBUG
    debug_mem_setup();
#endif
    list1 = linkedlist_new(int);
    list2 = linkedlist_new(struct foo);
}

void after_each()
{
    linkedlist_free(list1);
    linkedlist_free(list2);
}

void test_new()
{
    assert_equal(0, linkedlist_size(list1));
    assert_equal(0, linkedlist_size(list2));
}

void test_add()
{
    linkedlist_add(list1, -1);
    linkedlist_add(list1, -2);

    assert_equal(2, linkedlist_size(list1));
    assert_equal(-1, linkedlist_at(list1, 0));
    assert_equal(-2, linkedlist_at(list1, 1));

    linkedlist_add(list2, ((struct foo){-1, -1.0}));
    linkedlist_add(list2, ((struct foo){-2, -2.0}));

    assert_equal(2, linkedlist_size(list2));
    assert_struct_equal(((struct foo){-1, -1.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){-2, -2.0}), linkedlist_at(list2, 1));
}

void test_resize_up()
{
    for (size_t i = 0; i < 10; i++)
        linkedlist_add(list1, i);

    assert_equal(10, linkedlist_size(list1));
    for (size_t i = 0; i < 10; i++)
        assert_equal(i, linkedlist_at(list1, i));

    for (size_t i = 0; i < 10; i++)
        linkedlist_add(list2, ((struct foo){i, (float)i}));

    assert_equal(10, linkedlist_size(list2));
    for (size_t i = 0; i < 10; i++)
        assert_struct_equal(((struct foo){i, (float)i}), linkedlist_at(list2, i));
}

void test_is_empty()
{
    assert_true(linkedlist_is_empty(list1));
    linkedlist_add(list1, 1);
    assert_false(linkedlist_is_empty(list1));

    assert_true(linkedlist_is_empty(list2));
    linkedlist_add(list2, ((struct foo){1, 1.0}));
    assert_false(linkedlist_is_empty(list2));
}

void test_at()
{
    assert_panic(linkedlist_at(list1, 0));

    linkedlist_add(list1, 0);
    linkedlist_add(list1, 1);
    linkedlist_add(list1, 2);

    assert_equal(0, linkedlist_at(list1, 0));
    assert_equal(1, linkedlist_at(list1, 1));
    assert_equal(2, linkedlist_at(list1, 2));
    assert_panic(linkedlist_at(list1, 3));

    assert_equal(0, linkedlist_at(list1, -3));
    assert_equal(1, linkedlist_at(list1, -2));
    assert_equal(2, linkedlist_at(list1, -1));

    linkedlist_at(list1, 0) = -1;
    linkedlist_at(list1, 1) = -2;
    linkedlist_at(list1, 2) = -3;

    assert_equal(-1, linkedlist_at(list1, 0));
    assert_equal(-2, linkedlist_at(list1, 1));
    assert_equal(-3, linkedlist_at(list1, 2));

    assert_panic(linkedlist_at(list2, 0));

    linkedlist_add(list2, ((struct foo){0, 0.0}));
    linkedlist_add(list2, ((struct foo){1, 1.0}));
    linkedlist_add(list2, ((struct foo){2, 2.0}));

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_at(list2, 1));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_at(list2, 2));
    assert_panic(linkedlist_at(list2, 3));

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_at(list2, -3));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_at(list2, -2));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_at(list2, -1));

    linkedlist_at(list2, 0) = (struct foo){-1, -1.0};
    linkedlist_at(list2, 1) = (struct foo){-2, -2.0};
    linkedlist_at(list2, 2) = (struct foo){-3, -3.0};

    assert_struct_equal(((struct foo){-1, -1.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){-2, -2.0}), linkedlist_at(list2, 1));
    assert_struct_equal(((struct foo){-3, -3.0}), linkedlist_at(list2, 2));
}

void test_get_first()
{
    assert_panic(linkedlist_get_first(list1));

    linkedlist_add(list1, 1);
    assert_equal(1, linkedlist_get_first(list1));

    linkedlist_add_back(list1, 2);
    assert_equal(1, linkedlist_get_first(list1));

    linkedlist_add_front(list1, 0);
    assert_equal(0, linkedlist_get_first(list1));

    assert_panic(linkedlist_get_first(list2));

    linkedlist_add(list2, ((struct foo){1, 1.0}));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_get_first(list2));

    linkedlist_add_back(list2, ((struct foo){2, 2.0}));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_get_first(list2));

    linkedlist_add_front(list2, ((struct foo){0, 0.0}));
    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_get_first(list2));
}

void test_get_last()
{
    assert_panic(linkedlist_get_last(list1));

    linkedlist_add(list1, -1);
    assert_equal(-1, linkedlist_get_last(list1));

    linkedlist_add_front(list1, -2);
    assert_equal(-1, linkedlist_get_last(list1));

    linkedlist_add_back(list1, 0);
    assert_equal(0, linkedlist_get_last(list1));

    assert_panic(linkedlist_get_last(list2));

    linkedlist_add(list2, ((struct foo){1, 1.0}));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_get_last(list2));

    linkedlist_add_back(list2, ((struct foo){2, 2.0}));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_get_last(list2));

    linkedlist_add_front(list2, ((struct foo){0, 0.0}));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_get_last(list2));
}

void test_set_at()
{
    linkedlist_add(list1, 1);
    linkedlist_add(list1, 2);
    linkedlist_add(list1, 3);
    linkedlist_add(list1, 4);
    linkedlist_add(list1, 5);

    linkedlist_at(list1, 1) = -2;
    assert_equal(-2, linkedlist_at(list1, 1));

    linkedlist_at(list1, -2) = -4;
    assert_equal(-4, linkedlist_at(list1, 3));

    linkedlist_at(list1, -4) = 12;
    assert_equal(12, linkedlist_at(list1, 1));

    linkedlist_add(list2, ((struct foo){1, 1.0}));
    linkedlist_add(list2, ((struct foo){2, 2.0}));
    linkedlist_add(list2, ((struct foo){3, 3.0}));
    linkedlist_add(list2, ((struct foo){4, 4.0}));
    linkedlist_add(list2, ((struct foo){5, 5.0}));

    linkedlist_at(list2, 1) = (struct foo){-2, -2.0};
    assert_struct_equal(((struct foo){-2, -2.0}), linkedlist_at(list2, 1));

    linkedlist_at(list2, -2) = (struct foo){-4, -4.0};
    assert_struct_equal(((struct foo){-4, -4.0}), linkedlist_at(list2, 3));

    linkedlist_at(list2, -4) = (struct foo){12, 12.0};
    assert_struct_equal(((struct foo){12, 12.0}), linkedlist_at(list2, 1));
}

void test_add_at()
{
    linkedlist_add(list1, 0);
    linkedlist_add(list1, 2);

    linkedlist_add_at(list1, 1, 1);

    assert_equal(3, linkedlist_size(list1));

    assert_equal(0, linkedlist_at(list1, 0));
    assert_equal(1, linkedlist_at(list1, 1));
    assert_equal(2, linkedlist_at(list1, 2));

    assert_panic(linkedlist_add_at(list1, 3, 3));

    linkedlist_add(list2, ((struct foo){0, 0.0}));
    linkedlist_add(list2, ((struct foo){2, 2.0}));

    linkedlist_add_at(list2, 1, ((struct foo){1, 1.0}));

    assert_equal(3, linkedlist_size(list2));

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_at(list2, 1));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_at(list2, 2));

    assert_panic(linkedlist_add_at(list2, 3, ((struct foo){3, 3.0})));
}

void test_add_all()
{
    linkedlist_add_all(list1, 0, 1, 2);

    assert_equal(linkedlist_size(list1), 3);

    assert_equal(0, linkedlist_at(list1, 0));
    assert_equal(1, linkedlist_at(list1, 1));
    assert_equal(2, linkedlist_at(list1, 2));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    assert_equal(linkedlist_size(list2), 3);

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_at(list2, 1));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_at(list2, 2));
}

void test_literal()
{
    linkedlist_free(list1);
    list1 = linkedlist(int, 0, 1, 2);

    assert_equal(3, linkedlist_size(list1));

    assert_equal(0, linkedlist_at(list1, 0));
    assert_equal(1, linkedlist_at(list1, 1));
    assert_equal(2, linkedlist_at(list1, 2));

    linkedlist_free(list2);
    list2 = linkedlist(struct foo,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    assert_equal(3, linkedlist_size(list2));

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_at(list2, 1));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_at(list2, 2));
}

void test_pos_of()
{
    linkedlist_add_all(list1, 2, 15, -1);

    assert_equal(0, linkedlist_pos_of(list1, 2));
    assert_equal(1, linkedlist_pos_of(list1, 15));
    assert_equal(2, linkedlist_pos_of(list1, -1));
    assert_equal(-1, linkedlist_pos_of(list1, 1));

    linkedlist_add_all(list2,
                      ((struct foo){2, 2.0}),
                      ((struct foo){15, 15.0}),
                      ((struct foo){-1, -1.0}));

    assert_equal(0, linkedlist_pos_of(list2, ((struct foo){2, 2.0})));
    assert_equal(1, linkedlist_pos_of(list2, ((struct foo){15, 15.0})));
    assert_equal(2, linkedlist_pos_of(list2, ((struct foo){-1, -1.0})));
    assert_equal(-1, linkedlist_pos_of(list2, ((struct foo){1, 1.0})));
}

void test_remove()
{
    linkedlist_add_all(list1, 0, 1, 2);

    assert_equal(1, linkedlist_remove(list1, 1));
    assert_equal(0, linkedlist_at(list1, 0));
    assert_equal(2, linkedlist_at(list1, 1));

    assert_panic(linkedlist_remove(list1, -4));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_remove(list2, ((struct foo){1, 1.0})));
    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_at(list2, 0));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_at(list2, 1));

    assert_panic(linkedlist_remove(list2, ((struct foo){-4, -4.0})));
}

void test_remove_front()
{
    linkedlist_add_all(list1, 0, 1, 2);

    assert_equal(0, linkedlist_remove_front(list1));
    assert_equal(1, linkedlist_remove_front(list1));
    assert_equal(2, linkedlist_remove_front(list1));
    assert_panic(linkedlist_remove_front(list1));

    assert_equal(0, linkedlist_size(list1));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_remove_front(list2));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_remove_front(list2));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_remove_front(list2));
    assert_panic(linkedlist_remove_front(list2));

    assert_equal(0, linkedlist_size(list2));
}

void test_remove_back()
{
    linkedlist_add_all(list1, 0, 1, 2);

    assert_equal(2, linkedlist_remove_back(list1));
    assert_equal(1, linkedlist_remove_back(list1));
    assert_equal(0, linkedlist_remove_back(list1));
    assert_panic(linkedlist_remove_back(list1));

    assert_equal(0, linkedlist_size(list1));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_remove_back(list2));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_remove_back(list2));
    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_remove_back(list2));
    assert_panic(linkedlist_remove_back(list2));

    assert_equal(0, linkedlist_size(list2));
}

void test_remove_at()
{
    linkedlist_add_all(list1, 0, 1, 2);

    assert_equal(0, linkedlist_remove_at(list1, 0));
    assert_equal(2, linkedlist_remove_at(list1, 1));
    assert_equal(1, linkedlist_remove_at(list1, 0));

    assert_equal(0, linkedlist_size(list1));
    assert_panic(linkedlist_remove_at(list1, 0));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_remove_at(list2, 0));
    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_remove_at(list2, 1));
    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_remove_at(list2, 0));

    assert_equal(0, linkedlist_size(list2));
    assert_panic(linkedlist_remove_at(list2, 0));
}

void test_resize_down()
{
    linkedlist_free(list1);

    list1 = linkedlist_new(int);
    for (int i = 0; i < 20; i++)
        linkedlist_add(list1, i);

    for (int i = 19; i >= 0; i--)
        assert_equal(i, linkedlist_remove_back(list1));

    assert_equal(0, linkedlist_size(list1));

    linkedlist_free(list2);

    list2 = linkedlist_new(struct foo);
    for (int i = 0; i < 20; i++)
        linkedlist_add(list2, ((struct foo){i, (float)i}));

    for (int i = 19; i >= 0; i--)
        assert_struct_equal(((struct foo){i, (float)i}), linkedlist_remove_back(list2));

    assert_equal(0, linkedlist_size(list2));
}

void test_resize_up_and_down()
{
    linkedlist_free(list1);

    list1 = linkedlist_new(int);
    for (int i = 0; i < 20; i++)
        linkedlist_add(list1, i);

    for (int i = 19; i >= 0; i--)
        assert_equal(i, linkedlist_remove_back(list1));

    assert_equal(0, linkedlist_size(list1));

    linkedlist_free(list2);

    list2 = linkedlist_new(struct foo);
    for (int i = 0; i < 20; i++)
        linkedlist_add(list2, ((struct foo){i, (float)i}));

    for (int i = 19; i >= 0; i--)
        assert_struct_equal(((struct foo){i, (float)i}), linkedlist_remove_back(list2));

    assert_equal(0, linkedlist_size(list2));
}

void test_concat()
{
    linkedlist_t(int) first1 = linkedlist(int, 1, 2, 3);
    linkedlist_t(int) second1 = linkedlist(int, 4, 5, 6);

    linkedlist_t(int) concat1 = linkedlist_concat(first1, second1);
    assert_equal(6, linkedlist_size(concat1));

    for (int i = 0; i < 6; i++)
        assert_equal(i + 1, linkedlist_at(concat1, i));

    linkedlist_t(struct foo) first2 = linkedlist(struct foo,
                                               ((struct foo){1, 1.0}),
                                               ((struct foo){2, 2.0}),
                                               ((struct foo){3, 3.0}));
    linkedlist_t(struct foo) second2 = linkedlist(struct foo,
                                                ((struct foo){4, 4.0}),
                                                ((struct foo){5, 5.0}),
                                                ((struct foo){6, 6.0}));

    linkedlist_t(struct foo) concat2 = linkedlist_concat(first2, second2);
    assert_equal(6, linkedlist_size(concat2));

    for (int i = 0; i < 6; i++)
        assert_struct_equal(((struct foo){i + 1, (float)(i + 1)}),
                            linkedlist_at(concat2, i));
}

void *add_one(const void *x)
{
    return $box(*(int *)x + 1);
}

void *foo_add_one(const void *x)
{
    const struct foo *X = x;
    return $new(struct foo, .i = X->i + 1, .f = X->f + 1);
}

void test_map()
{
    linkedlist_add_all(list1, 0, 1, 2);
    linkedlist_t(int) new_list1 = linkedlist_map(list1, add_one);

    assert_equal(3, linkedlist_size(new_list1));

    for (int i = 0; i < 3; i++)
        assert_equal(i + 1, linkedlist_at(new_list1, i));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    linkedlist_t(struct foo) new_list2 = linkedlist_map(list2, foo_add_one);

    assert_equal(3, linkedlist_size(new_list2));

    for (int i = 0; i < 3; i++)
        assert_struct_equal(((struct foo){i + 1, (float)(i + 1)}),
                            linkedlist_at(new_list2, i));
}

bool only_positive(const void *x)
{
    return *(int *)x > 0;
}

bool foo_only_positive(const void *x)
{
    const struct foo *X = x;
    return X->i > 0 && X->f > 0;
}

void test_filter()
{
    linkedlist_add_all(list1, -2, -1, 0, 1, 2);
    linkedlist_t(int) new_list1 = linkedlist_filter(list1, only_positive);

    assert_equal(2, linkedlist_size(new_list1));

    for (int i = 1; i <= 2; i++)
        assert_equal(i, linkedlist_at(new_list1, i - 1));

    linkedlist_add_all(list2,
                      ((struct foo){-2, -2.0}),
                      ((struct foo){-1, -1.0}),
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    linkedlist_t(struct foo) new_list2 = linkedlist_filter(list2, foo_only_positive);

    assert_equal(2, linkedlist_size(new_list2));

    for (int i = 1; i <= 2; i++)
        assert_struct_equal(((struct foo){i, (float)i}),
                            linkedlist_at(new_list2, i - 1));
}

void *sum(const void *a, const void *b)
{
    return $box(*(int *)a + *(int *)b);
}

void *foo_sum(const void *a, const void *b)
{
    const struct foo *A = a;
    const struct foo *B = b;
    return $new(struct foo, .i = A->i + B->i, .f = A->f + B->f);
}

void test_reduce()
{
    linkedlist_free(list1);
    list1 = linkedlist(int, 0, 1, 2, 3, 4, 5);
    assert_equal(15, linkedlist_reduce(list1, sum));

    linkedlist_free(list1);
    list1 = linkedlist(int, 0, 1, 2, 3, 4, 5, 6);
    assert_equal(21, linkedlist_reduce(list1, sum));

    linkedlist_free(list2);
    list2 = linkedlist(struct foo,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}),
                      ((struct foo){3, 3.0}),
                      ((struct foo){4, 4.0}),
                      ((struct foo){5, 5.0}));
    assert_struct_equal(((struct foo){15, 15.0}), linkedlist_reduce(list2, foo_sum));

    linkedlist_free(list2);
    list2 = linkedlist(struct foo,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}),
                      ((struct foo){3, 3.0}),
                      ((struct foo){4, 4.0}),
                      ((struct foo){5, 5.0}),
                      ((struct foo){6, 6.0}));
    assert_struct_equal(((struct foo){21, 21.0}), linkedlist_reduce(list2, foo_sum));
}

void test_equal()
{
    linkedlist_free(list1);

    list1 = linkedlist(int, 0, 1, 2);
    linkedlist_t(int) list12 = linkedlist(int, 0, 1, 2);
    linkedlist_t(int) list13 = linkedlist(int, 0, 1, 2, 3);
    linkedlist_t(int) list14 = linkedlist(int, 1, -2, -5);

    assert_true(linkedlist_equal(list1, list12));
    assert_false(linkedlist_equal(list1, list13));
    assert_false(linkedlist_equal(list1, list14));

    linkedlist_free(list12);
    linkedlist_free(list13);
    linkedlist_free(list14);

    list2 = linkedlist(struct foo,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));
    linkedlist_t(struct foo) list22 = linkedlist(struct foo,
                                               ((struct foo){0, 0.0}),
                                               ((struct foo){1, 1.0}),
                                               ((struct foo){2, 2.0}));
    linkedlist_t(struct foo) list23 = linkedlist(struct foo,
                                               ((struct foo){0, 0.0}),
                                               ((struct foo){1, 1.0}),
                                               ((struct foo){2, 2.0}),
                                               ((struct foo){3, 3.0}));
    linkedlist_t(struct foo) list24 = linkedlist(struct foo,
                                               ((struct foo){0, 0.0}),
                                               ((struct foo){-2, -2.0}),
                                               ((struct foo){-5, -5.0}));

    assert_true(linkedlist_equal(list2, list22));
    assert_false(linkedlist_equal(list2, list23));
    assert_false(linkedlist_equal(list2, list24));

    linkedlist_free(list22);
    linkedlist_free(list23);
    linkedlist_free(list24);
}

bool _strcmp(const void *a, const void *b)
{
    return strcmp((char *)a, (char *)b) == 0;
}

void test_equal_item_eq_fn()
{
    linkedlist_t(char *) listc1 = linkedlist(char *, "string1", "string2", "string3");
    linkedlist_t(char *) listc2 = linkedlist(char *, "string1", "string2", "string3");
    linkedlist_t(char *) listc3 = linkedlist(char *, "string1", "string2", "not string 3");

    assert_true(linkedlist_equal(listc1, listc2, .eq_fn = (equal_fn_t)_strcmp));
    assert_false(linkedlist_equal(listc1, listc3, .eq_fn = (equal_fn_t)_strcmp));
}

void test_ref_forward_iter()
{
    linkedlist_add_all(list1, 0, 1, 2);
    ref1 = linkedlist_ref(list1);

    assert_equal(0, linkedlist_ref_get_item(ref1));
    assert_true(linkedlist_ref_has_next(ref1));
    linkedlist_ref_next(ref1);

    assert_equal(1, linkedlist_ref_get_item(ref1));
    assert_true(linkedlist_ref_has_next(ref1));
    linkedlist_ref_next(ref1);

    assert_equal(2, linkedlist_ref_get_item(ref1));
    assert_false(linkedlist_ref_has_next(ref1));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));
    ref2 = linkedlist_ref(list2);

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_ref_get_item(ref2));
    assert_true(linkedlist_ref_has_next(ref2));
    linkedlist_ref_next(ref2);

    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_ref_get_item(ref2));
    assert_true(linkedlist_ref_has_next(ref2));
    linkedlist_ref_next(ref2);

    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_ref_get_item(ref2));
    assert_false(linkedlist_ref_has_next(ref2));
}

void test_ref_for_loop()
{
    linkedlist_add_all(list1, 0, 1, 2);
    int i;

    i = 0;
    for (ref1 = linkedlist_ref(list1);
         linkedlist_ref_is_valid(ref1);
         linkedlist_ref_next(ref1), i++)
    {
        assert_equal(i, linkedlist_ref_get_item(ref1));
    }

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));

    i = 0;
    for (ref2 = linkedlist_ref(list2);
         linkedlist_ref_is_valid(ref2);
         linkedlist_ref_next(ref2), i++)
    {
        assert_struct_equal(((struct foo){i, (float)i}), linkedlist_ref_get_item(ref2));
    }
}

void test_ref_backwards_iter()
{
    linkedlist_add_all(list1, 0, 1, 2);
    ref1 = linkedlist_ref(list1);

    for (int i = 0; i < 2; i++)
        linkedlist_ref_next(ref1);

    assert_equal(2, linkedlist_ref_get_item(ref1));
    assert_true(linkedlist_ref_has_prev(ref1));
    linkedlist_ref_prev(ref1);

    assert_equal(1, linkedlist_ref_get_item(ref1));
    assert_true(linkedlist_ref_has_prev(ref1));
    linkedlist_ref_prev(ref1);

    assert_equal(0, linkedlist_ref_get_item(ref1));
    assert_false(linkedlist_ref_has_prev(ref1));

    linkedlist_add_all(list2,
                      ((struct foo){0, 0.0}),
                      ((struct foo){1, 1.0}),
                      ((struct foo){2, 2.0}));
    ref2 = linkedlist_ref(list2);

    for (int i = 0; i < 2; i++)
        linkedlist_ref_next(ref2);

    assert_struct_equal(((struct foo){2, 2.0}), linkedlist_ref_get_item(ref2));
    assert_true(linkedlist_ref_has_prev(ref2));
    linkedlist_ref_prev(ref2);

    assert_struct_equal(((struct foo){1, 1.0}), linkedlist_ref_get_item(ref2));
    assert_true(linkedlist_ref_has_prev(ref2));
    linkedlist_ref_prev(ref2);

    assert_struct_equal(((struct foo){0, 0.0}), linkedlist_ref_get_item(ref2));
    assert_false(linkedlist_ref_has_prev(ref2));
}

int main(int argc, char *argv[])
{
    TEST_SUITE(
        TEST(test_new),
        TEST(test_add),
        TEST(test_resize_up),
        TEST(test_is_empty),
        TEST(test_at),
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