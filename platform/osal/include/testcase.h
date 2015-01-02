/*
 * testcase.h
 *
 *  Created on: Dec 10, 2014
 *      Author: rwl
 */

#ifndef OSAL_INCLUDE_TESTCASE_H_
#define OSAL_INCLUDE_TESTCASE_H_
#include <osal.h>

#define PRINT(OE,MSG,...) {\
	char ___m[90] = {0}; \
	osal_sprintf(___m,(MSG),##__VA_ARGS__); \
	(OE)->p(___m);}


typedef bool (*test_function)(OE oe);
typedef struct _test_ {
	char * name;
	test_function f;
} Test;

struct _test_suit_;
typedef struct _test_suit_ * tsptr;



struct _test_suit_;
typedef struct _test_suit_ {
	char * name;
	struct _test_suit_ * subs;
	uint lsubs;
	Test * tests;
	uint ltest;
} TestSuit;

bool run_test_suit(OE oe, TestSuit suit);

#define TEST_MAIN(TOPSUIT)\
	int main(int c, char **a) { \
		OE oe = OperatingEnvironment_LinuxNew();\
		PRINT(oe,"TestCase Running Suits: ");\
		run_test_suit(oe,(TOPSUIT)); \
		OperatingEnvironment_LinuxDestroy(&oe); \
		return 0;\
    }

#endif /* OSAL_INCLUDE_TESTCASE_H_ */
