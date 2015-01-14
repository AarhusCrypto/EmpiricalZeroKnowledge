/*
 * test_rtz14.c
 *
 *  Created on: Jan 14, 2015
 *      Author: rwl
 */


#include <osal.h>
#include <rtz14.h>
#include "test.h"
#include <rnd.h>
#include <circuitparser.h>
#include <circuit_analyser.h>

static int test_create_rtz14(OE oe) {
	_Bool ok = 0;
	Rnd rnd = LibcWeakRandomSource_New(oe);
	Rtz14 rtz = Rtz14_New(oe, rnd);
	ok = (rtz != 0);
	Rtz14_Destroy(&rtz);
	ok &= (rtz == 0);
	return ok;
}

static int test_run_proof_4_and(OE oe) {
	_Bool ok = 0;


	List circuit = SingleLinkedList_new(oe);
	byte witness[1] = {1};
	Rnd rnd = LibcWeakRandomSource_New(oe);
	Rtz14 rtz = Rtz14_New(oe,rnd);

	struct _gate_ g = {0};
	Gate gp = &g;
	g.dst =0;
	g.op1=1;
	g.op2=2;
	circuit->add_element(gp);

	ok = (rtz != 0);
	ok &= rtz->executeProof(circuit,witness,0,2020);

	Rtz14_Destroy(&rtz);
	SingleLinkedList_destroy(&circuit);
	LibcWeakRandomSource_Destroy(&rnd);

	return ok;
}

Test tests[] = {
		{"creating an RTZ14 instance",test_create_rtz14},
		{"", test_run_proof_4_and}
};

TEST_MAIN

