/*
 * circuittool.c
 *
 *  Created on: Feb 1, 2015
 *      Author: rwl
 */

#include <osal.h>


int main() {
	OE oe = OperatingEnvironment_LinuxNew();

	OperatingEnvironment_LinuxDestroy(&oe);
	return 0;
}
