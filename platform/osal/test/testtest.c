/*
 * testtest.c
 *
 *  Created on: Dec 10, 2014
 *      Author: rwl
 */
// TODO(rwz): We need to have a structured way of doing tests.


#include <testcase.h>

extern test_function testoe;

test_function suit[] = {};

TestSuit toptest = {
		"Operating Environment Abstraction Layer test suit.",
		0,
		0,
		0,
		0
};

TEST_MAIN(toptest)
