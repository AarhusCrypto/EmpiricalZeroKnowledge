/*
 * circuittool.c
 *
 *  Created on: Feb 1, 2015
 *      Author: rwl
 */

#include <osal.h>


int main() {
	OE oe = OperatingEnvironment_New();

	OperatingEnvironment_Destroy(&oe);
	return 0;
}
