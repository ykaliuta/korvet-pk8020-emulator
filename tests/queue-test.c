/*
 *  Copyright (c) 2018 Yauheni Kaliuta
 */

#include <cgreen/cgreen.h>
#include <queue.h>

#define NAME "queue"

static void setup(void)
{
}

static void teardown(void)
{
}

Ensure(one_element_size_creates_empty_queue)
{
	struct queue *q;
	int el = 2;
	bool rc;

	q = queue_new(1, sizeof(el));
	assert_that(q, is_not_null);

	rc = queue_is_empty(q);
	assert_that(rc, is_true);

	queue_destroy(q);
}

Ensure(one_element_size_creates_nonfull_queue)
{
	struct queue *q;
	int el = 2;
	bool rc;

	q = queue_new(1, sizeof(el));
	assert_that(q, is_not_null);

	rc = queue_is_full(q);
	assert_that(rc, is_false);

	queue_destroy(q);
}

Ensure(one_element_size_accepts_one_element)
{
	struct queue *q;
	int el = 2;
	void *rc;

	q = queue_new(1, sizeof(el));
	assert_that(q, is_not_null);

	rc = queue_push(q, &el);
	assert_that(rc, is_not_null);

	queue_destroy(q);
}

Ensure(one_element_size_full_after_one_element)
{
	struct queue *q;
	int el = 2;
	bool rc;

	q = queue_new(1, sizeof(el));
	assert_that(q, is_not_null);

	queue_push(q, &el);
	rc = queue_is_full(q);
	assert_that(rc, is_true);

	queue_destroy(q);
}

Ensure(three_elements_are_the_same_after_read)
{
	struct queue *q;
	int el1 = 1;
	int el2 = 2;
	int el3 = 3;
	int el = 0;
	bool rc;

	q = queue_new(3, sizeof(el));
	assert_that(q, is_not_null);

	queue_push(q, &el1);
	queue_push(q, &el2);
	queue_push(q, &el3);
	rc = queue_is_full(q);
	assert_that(rc, is_true);

	queue_pop(q, &el);
	assert_that(el, is_equal_to(1));
	queue_pop(q, &el);
	assert_that(el, is_equal_to(2));
	queue_pop(q, &el);
	assert_that(el, is_equal_to(3));

	rc = queue_is_empty(q);
	assert_that(rc, is_true);

	queue_destroy(q);
}

TestSuite *my_create_test_suite(void)
{
	TestSuite *suite = create_named_test_suite(NAME);

	set_setup(suite, setup);

	add_test(suite, one_element_size_creates_empty_queue);
	add_test(suite, one_element_size_creates_nonfull_queue);
	add_test(suite, one_element_size_accepts_one_element);
	add_test(suite, one_element_size_full_after_one_element);
	add_test(suite, three_elements_are_the_same_after_read);

	set_teardown(suite, teardown);

	return suite;
}
