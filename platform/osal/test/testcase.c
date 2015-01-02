/*
 * testcase.c
 *
 *  Created on: Dec 10, 2014
 *      Author: rwl
 */
#include <testcase.h>

// TODO(rwz): we need to implement this properly
bool run_test_suit(OE oe, TestSuit suit) {
	bool success = True;
	uint i = 0;
/*
	PRINT(oe,"[SUIT] %s",suit.name);

	for(i = 0;i < sizeof(suit.subs)/sizeof(char *);++i) {
		TestSuit * next = (TestSuit *)suit.subs[i];
		if (!run_test_suit(oe,*next)) {
			return False;
		}
	}

	for(i = 0; i < suit.ltest;++i) {
		bool r = suit.tests[i].f(oe);
		if (r == True) {
			PRINT(oe, "[TEST] %s [OK]",suit.tests[i].name);
		} else {
			PRINT(oe, "[TEST] %s [FAILED]",suit.tests[i].name);
			success = False;
		}
	}
	*/
	return success;
}

