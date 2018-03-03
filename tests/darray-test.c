/*
 *  Copyright (c) 2018 Yauheni Kaliuta
 */

#include <cgreen/cgreen.h>
#include <darray.h>

#define NAME "darray"

static void setup(void)
{
}

static void teardown(void)
{
}

Ensure(one_element_size_creates_empty_darray)
{
	struct darray *a;
	bool rc;
	int el = 2;

	a = darray_new(1, sizeof(el));
	assert_that(a, is_not_null);

	rc = darray_is_empty(a);
	assert_that(rc, is_true);

	darray_destroy(a);
}

Ensure(three_elements_are_the_same_after_read)
{
	struct darray *a;
	int el1 = 1;
	int el2 = 2;
	int el3 = 3;
	int el = 0;
	bool rc;

	a = darray_new(3, sizeof(el));
	assert_that(a, is_not_null);

	darray_push(a, &el1);
	darray_push(a, &el2);
	darray_push(a, &el3);

	darray_read(a, 0, &el);
	assert_that(el, is_equal_to(1));
	darray_read(a, 1, &el);
	assert_that(el, is_equal_to(2));
	darray_read(a, 2, &el);
	assert_that(el, is_equal_to(3));

	darray_destroy(a);
}

Ensure(one_element_accepts_three_and_preserves_them)
{
	struct darray *a;
	int el1 = 1;
	int el2 = 2;
	int el3 = 3;
	int el = 0;
	bool rc;

	a = darray_new(1, sizeof(el));
	assert_that(a, is_not_null);

	darray_push(a, &el1);
	darray_push(a, &el2);
	darray_push(a, &el3);

	darray_read(a, 0, &el);
	assert_that(el, is_equal_to(1));
	darray_read(a, 1, &el);
	assert_that(el, is_equal_to(2));
	darray_read(a, 2, &el);
	assert_that(el, is_equal_to(3));

	darray_destroy(a);
}

TestSuite *my_create_test_suite(void)
{
	TestSuite *suite = create_named_test_suite(NAME);

	set_setup(suite, setup);

	add_test(suite, one_element_size_creates_empty_darray);
	add_test(suite, three_elements_are_the_same_after_read);
	add_test(suite, one_element_accepts_three_and_preserves_them);

	set_teardown(suite, teardown);

	return suite;
}
