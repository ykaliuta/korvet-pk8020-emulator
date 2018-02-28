/*
 *  Copyright (c) 2018 Yauheni Kaliuta
 */

#include <cgreen/cgreen.h>

TestSuite *my_create_test_suite();

int main(int argc, char **argv)
{
	TestSuite *suite = my_create_test_suite();
	TestReporter *reporter = create_text_reporter();
	int i;
	int rc;

	if (argc <= 1)
		return run_test_suite(suite, reporter);

	for (i = 1; i < argc; i++) {
		rc = run_single_test(suite, argv[i], reporter);
		if (rc != 0)
			return rc;
	}
	return 0;
}
