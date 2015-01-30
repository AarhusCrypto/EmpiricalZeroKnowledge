/*
 * test.h
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */

#ifndef RTZ14_TEST_TEST_H_
#define RTZ14_TEST_TEST_H_
#include <osal.h>
#include <datetime.h>
typedef struct _test_ {
	char * name;
	int (*run)(OE oe);
} Test;


#define DBG_P(OE,FMT,...) {\
	char _________b[512] = {0}; \
	osal_sprintf(_________b,FMT,##__VA_ARGS__);\
	(OE)->p(_________b);\
}
#define AssertTrue(COND) \
	if (( ok &= (COND)) != 1) goto test_end;

#define TEST_MAIN \
		int main(int c, char ** a) {\
			char b[42] = {0};\
			uint i = 0;\
			uint failed_tests = 0;\
			ull start = 0; \
			OE oe = OperatingEnvironment_LinuxNew();\
			DateTime dt = DateTime_New(oe);\
			osal_sprintf(b,"Running %d tests.",sizeof(tests)/sizeof(Test));\
			oe->p(b);\
			start = dt->getMilliTime();\
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
			zeromem(b,42);\
			osal_sprintf(b,"%u test where %u failed ran in %lu ms",sizeof(tests)/sizeof(Test),failed_tests,dt->getMilliTime()-start);\
			oe->p(b);\
			return failed_tests;\
		}


#endif /* RTZ14_TEST_TEST_H_ */
