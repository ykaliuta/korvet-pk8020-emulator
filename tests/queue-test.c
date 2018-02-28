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

TestSuite *my_create_test_suite(void)
{
	TestSuite *suite = create_named_test_suite(NAME);

	set_setup(suite, setup);

	add_test(suite, one_element_size_creates_empty_queue);
	add_test(suite, one_element_size_creates_nonfull_queue);
	add_test(suite, one_element_size_accepts_one_element);
	add_test(suite, one_element_size_full_after_one_element);

	set_teardown(suite, teardown);

	return suite;
}
