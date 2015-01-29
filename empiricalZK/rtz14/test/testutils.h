/*
 * testutils.h
 *
 *  Created on: Jan 28, 2015
 *      Author: rwl
 */

#ifndef RTZ14_TEST_TESTUTILS_H_
#define RTZ14_TEST_TESTUTILS_H_

#include <rnd.h>

void TestRnd_Destroy(Rnd * tr);

Rnd TestRnd_New(OE oe, byte v);

#endif /* RTZ14_TEST_TESTUTILS_H_ */
