/*
 * testutils.c
 *
 *  Created on: Jan 28, 2015
 *      Author: rwl
 */
#include <testutils.h>
#include <coov4.h>

// random source for test purposes that always generates sequences of {v}.
typedef struct _test_rnd_ {
	OE oe;
	byte v;
} * TestRnd;

// fill byte array {d} with {ld} values of {impl->v}
COO_DEF(Rnd,void,test_rnd_rand,byte * d, uint ld) {
	uint i = 0;
	TestRnd impl = this->impl;
	for(i = 0;i < ld;++i) d[i] = impl->v;
}}

// create a test random source always producing {v}
Rnd TestRnd_New(OE oe, byte v) {
	Rnd rnd = oe->getmem(sizeof(*rnd));
	TestRnd tr = 0;

	if (!rnd) return 0;

	tr = oe->getmem(sizeof(*tr));
	if (!tr) goto fail;

	rnd->rand = COO_attach(rnd,Rnd_test_rnd_rand);
	rnd->impl = tr;
	tr->oe = oe;
	tr->v = v;

	return rnd;
	fail:

	TestRnd_Destroy(&rnd);
	if (rnd) {
		oe->putmem(rnd);
	}
	return 0;
}

void TestRnd_Destroy(Rnd * tr) {
	Rnd r = 0;
	TestRnd t = 0;
	OE oe =0;

	if (!tr) return;
	r = *tr;

	t = r->impl;
	if (!t) return;

	COO_detach(r->rand);
	oe = t->oe;

	oe->putmem(r);
	oe->putmem(t);

	*tr = 0;
}


