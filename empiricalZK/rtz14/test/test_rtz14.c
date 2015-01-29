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
#include <commitment.h>
#include "testutils.h"


static int test_create_rtz14(OE oe) {
	_Bool ok = 0;
	Rnd rnd = LibcWeakRandomSource_New(oe);
	CommitmentScheme cs = DummyScheme_New(oe);
	Rtz14 rtz = Rtz14_New(oe, rnd, cs,0);
	ok = (rtz != 0);
	Rtz14_Destroy(&rtz);
	ok &= (rtz == 0);
	return ok;
}

static int test_run_proof_4_and(OE oe) {
	_Bool ok = 0;
	CommitmentScheme cs = DummyScheme_New(oe);

	List circuit = SingleLinkedList_new(oe);
	byte witness[1] = {1};
	Rnd rnd = LibcWeakRandomSource_New(oe);
	Rtz14 rtz = Rtz14_New(oe,rnd,cs,0);

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

typedef struct _test_args_ {
	OE oe;
	Rtz14 rtz;
	List circuit;
} TestArgs;

static void * test_run_proof_one_and_verifier(TestArgs * args) {
	uint i = 1;
	args->oe->p("Verfier thread is alive.");
	while(i++ < 1073741824/4);;
	args->rtz->executeProof(args->circuit,0,"127.0.0.1",2020);
	return 0;
}
static int test_run_proof_one_and(OE oe) {
	_Bool ok = 1;
	CommitmentScheme cs = 0;
	Rnd rnd = 0;
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	uint aoo = 0; // address of one
	byte witness = 0x07;
	Rtz14 rtz_prover = 0, rtz_verifier = 0;
	List circuit = 0;
	TestArgs args = {0};
	ThreadID tid = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe,tk);
	cs = DummyScheme_New(oe);
	rnd = LibcWeakRandomSource_New(oe);
	rtz_prover = Rtz14_New(oe, rnd, cs, aoo);
	rtz_verifier = Rtz14_New(oe, rnd, cs, aoo);

	circuit = cp->parseSource("AND(2,0,1)",11);

	args.circuit = circuit;
	args.oe = oe;
	args.rtz = rtz_verifier;


	AssertTrue(rtz_prover != 0);
	AssertTrue(rtz_verifier != 0);

	tid = oe->newthread(test_run_proof_one_and_verifier,&args);
	rtz_prover->executeProof(circuit,&witness,0,2020);
	oe->jointhread(tid);

	test_end:
	return ok;
}

static int test_run_proof_one_and_fail(OE oe) {
	_Bool ok = 1;
	CommitmentScheme cs = 0;
	Rnd rnd = 0;
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	uint aoo = 0; // address of one
	byte witness = 0x03; // 1 1
	Rtz14 rtz_prover = 0, rtz_verifier = 0;
	List circuit = 0;
	TestArgs args = {0};
	ThreadID tid = 0;

	tk = FunCallTokenizer_New(oe);
	AssertTrue(tk != 0)
	cp = CircuitParser_New(oe,tk);
	AssertTrue(cp != 0)
	cs = DummyScheme_New(oe);
	AssertTrue(cs != 0)
	rnd = (Rnd)TestRnd_New(oe,0xAA);
	AssertTrue(rnd != 0)
	rtz_prover = Rtz14_New(oe, rnd, cs, aoo);
	AssertTrue(rtz_prover != 0)
	rtz_verifier = Rtz14_New(oe, rnd, cs, aoo);
	AssertTrue( rtz_verifier != 0 )

	// adr 0, adr 2
	// 1      1
	circuit = cp->parseSource("INV(1,0,0);AND(3,1,2,0);",24);

	// adr 0, adr 1
	// 1      1
	//args.circuit = cp->parseSource("INV(2,0,0);AND(3,1,2);",26);
	args.circuit = cp->parseSource("INV(1,0,0);AND(3,2,1,0);",24);
	args.oe = oe;
	args.rtz = rtz_verifier;


	AssertTrue(rtz_prover != 0);
	AssertTrue(rtz_verifier != 0);

	tid = oe->newthread(test_run_proof_one_and_verifier,&args);
	AssertTrue( ok = rtz_prover->executeProof(circuit,&witness,0,2020) == False );
	oe->jointhread(tid);

	test_end:
	return ok;
}


static int test_run_proof_randomness_fixed_to_zero(OE oe) {
	_Bool ok = 1;
	CommitmentScheme cs = 0;
	Rnd rnd = 0;
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	uint aoo = 0; // address of one
	byte witness = 0x03; // 1 1
	Rtz14 rtz_prover = 0, rtz_verifier = 0;
	List circuit = 0;
	TestArgs args = {0};
	ThreadID tid = 0;

	tk = FunCallTokenizer_New(oe);
	AssertTrue(tk != 0)
	cp = CircuitParser_New(oe,tk);
	AssertTrue(cp != 0)
	cs = DummyScheme_New(oe);
	AssertTrue(cs != 0)
	rnd = (Rnd)TestRnd_New(oe,0x00);
	AssertTrue(rnd != 0)
	rtz_prover = Rtz14_New(oe, rnd, cs, aoo);
	AssertTrue(rtz_prover != 0)
	rtz_verifier = Rtz14_New(oe, rnd, cs, aoo);
	AssertTrue( rtz_verifier != 0 )

	// adr 0, adr 2
	// 1      1
	circuit = cp->parseSource("INV(1,0,0);AND(3,1,2,0);",24);

	// adr 0, adr 1
	// 1      1
	//args.circuit = cp->parseSource("INV(2,0,0);AND(3,1,2);",26);
	args.circuit = cp->parseSource("INV(1,0,0);AND(3,2,1,0);",24);
	args.oe = oe;
	args.rtz = rtz_verifier;


	AssertTrue(rtz_prover != 0);
	AssertTrue(rtz_verifier != 0);

	tid = oe->newthread(test_run_proof_one_and_verifier,&args);
	AssertTrue( ok = rtz_prover->executeProof(circuit,&witness,0,2020) == True );
	oe->jointhread(tid);

	test_end:
	return ok;
}

static int run_protocol_on_aes(OE oe) {
	_Bool ok = 1;
	CommitmentScheme cs = 0;
	Rnd rnd = 0;
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	uint aoo = 256; // address of one
	byte witness[257] = {0};
	Rtz14 rtz_prover = 0, rtz_verifier = 0;
	List circuit = 0;
	TestArgs args = {0};
	ThreadID tid = 0;
	byte * buf = 0;
	uint lbuf = 1060365;
	uint fp = 0;
	witness[256] = 1;
	buf = oe->getmem(lbuf);
	AssertTrue(buf != 0);
	fp = oe->open("file ../test/AES");
	oe->read(fp,buf,&lbuf);
	oe->close(fp);

	tk = FunCallTokenizer_New(oe);
	AssertTrue(tk != 0)
	cp = CircuitParser_New(oe,tk);
	AssertTrue(cp != 0)
	cs = DummyScheme_New(oe);
	AssertTrue(cs != 0)
	rnd = (Rnd)TestRnd_New(oe,0x00);
	AssertTrue(rnd != 0)
	rtz_prover = Rtz14_New(oe, rnd, cs, aoo);
	AssertTrue(rtz_prover != 0)
	rtz_verifier = Rtz14_New(oe, rnd, cs, aoo);
	AssertTrue( rtz_verifier != 0 )



	circuit = cp->parseSource(buf,lbuf);

	args.circuit = cp->parseSource(buf,lbuf);
	args.oe = oe;
	args.rtz = rtz_verifier;


	AssertTrue(rtz_prover != 0);
	AssertTrue(rtz_verifier != 0);

	tid = oe->newthread(test_run_proof_one_and_verifier,&args);
	AssertTrue( ok = rtz_prover->executeProof(circuit,&witness,0,2020) == True );
	oe->jointhread(tid);

	test_end:
	return ok;

}

Test tests[] = {
	//	{"creating an RTZ14 instance",test_create_rtz14},
	//	{"creting rtz14 with witness (as prover)", test_run_proof_4_and},
		{"run protocol prover one and-gate.",test_run_proof_one_and},
		{"run protocol prover one and-gate fails",test_run_proof_one_and_fail},
		{"run protocol prover one and-gate should fail but randomness is zero",
		 test_run_proof_randomness_fixed_to_zero},
		{"run protocol on AES",run_protocol_on_aes},

};

TEST_MAIN

