/*
 * test_circuitparser.c
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */

#include <osal.h>
#include <circuitparser.h>
#include "test.h"


static int create_fun_circuit_parser(OE oe) {
	CircuitParser cp = CircuitParser_New(oe,"");

	return cp != 0;
}

static int parse_correct_xor(OE oe) {
	CircuitParser cp = CircuitParser_New(oe,"XOR(0,0,0)");

}

static int parse_correct_and(OE oe) {

}

static int parse_correct_inv(OE oe) {

}

Test tests[] = {
		{"Creation of function based circuit parser", create_fun_circuit_parser},
		{"Parse correct XOR",parse_correct_xor},
		{"parse correct AND",parse_correct_and},
		{"parse_correct_INV",parse_correct_inv}
};

TEST_MAIN
