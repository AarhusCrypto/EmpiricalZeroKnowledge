/*
 * test.h
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */

#ifndef RTZ14_TEST_TEST_H_
#define RTZ14_TEST_TEST_H_
#include <osal.h>
typedef struct _test_ {
	char * name;
	int (*run)(OE oe);
} Test;


#define DBG_P(OE,FMT,...) {\
	char _________b[512] = {0}; \
	osal_sprintf(_________b,FMT,##__VA_ARGS__);\
	(OE)->p(_________b);\
}

#define TEST_MAIN \
		int main(int c, char ** a) {\
			char b[42] = {0};\
			uint i = 0;\
			uint failed_tests = 0;\
			OE oe = OperatingEnvironment_LinuxNew();\
			osal_sprintf(b,"Running %d tests.",sizeof(tests)/sizeof(Test));\
			oe->p(b);\
			for(i = 0; i < sizeof(tests)/sizeof(Test);++i) {\
				char buf[512] = {0};\
				int res = tests[i].run(oe);\
				if (res) {\
					osal_sprintf(buf,"Test \"%s\"\t ... [ OK ]",tests[i].name);\
					oe->p(buf);\
				} else {\
					osal_sprintf(buf,"Test \"%s\"\t ... [ FAILED ]",tests[i].name);\
					oe->syslog(OSAL_LOGLEVEL_FATAL,buf);\
					++failed_tests;\
				}\
			}\
			return failed_tests;\
		}


#endif /* RTZ14_TEST_TEST_H_ */
