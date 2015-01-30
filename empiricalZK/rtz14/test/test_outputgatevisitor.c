/*
 * test_outputgatevisitor.c
 *
 *  Created on: Jan 29, 2015
 *      Author: rwl
 */



#include "test.h"
#include "testutils.h"
#include <circuit_analyser.h>
#include <circuitparser.h>
#include <fs.h>

static int create_ogv(OE oe) {
	_Bool ok = 1;
	CircuitVisitor ogv = OutputGateVisitor_New(oe);

	AssertTrue(ogv != 0)

	OutputGateVisitor_Destroy(&ogv);

	AssertTrue(ogv == 0);

	test_end:
	return ok;
}

static int test_run_one_and_gate(OE oe) {
	_Bool ok = 1;
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	CircuitVisitor ogv = 0;
	List circuit = 0;
	List output_gates = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);

	// zero is written but never read
	circuit = cp->parseSource("AND(0,1,2,0);",14);
	AssertTrue(circuit != 0)

	ogv = OutputGateVisitor_New(oe);
	AssertTrue(ogv != 0)

	output_gates = ogv->visit(circuit);
	AssertTrue(output_gates != 0)
	AssertTrue(output_gates->size() == 1)
	AssertTrue( ((ull)output_gates->get_element(0)) == 0 )

test_end:
	SingleLinkedList_destroy(&output_gates);
	OutputGateVisitor_Destroy(&ogv);
	Circuit_Destroy(oe,&circuit);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	return ok;
}


static int test_run_one_xor_gate(OE oe) {
	_Bool ok = 1;
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	CircuitVisitor ogv = 0;
	List circuit = 0;
	List output_gates = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);

	// zero is written but never read
	circuit = cp->parseSource("XOR(0,1,0,0);",14);
	AssertTrue(circuit != 0)

	ogv = OutputGateVisitor_New(oe);
	AssertTrue(ogv != 0)

	output_gates = ogv->visit(circuit);
	AssertTrue(output_gates != 0)
	AssertTrue(output_gates->size() == 1)
	AssertTrue( ((ull)output_gates->get_element(0)) == 0 )

test_end:
	SingleLinkedList_destroy(&output_gates);
	OutputGateVisitor_Destroy(&ogv);
	Circuit_Destroy(oe,&circuit);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	return ok;
}

static int test_with_aes(OE oe) {
	_Bool ok = 1;
	uint lbuf = read_file_size("../test/AES");
	byte * buf = oe->getmem(lbuf);
	uint fp = oe->open("file ../test/AES");
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	CircuitVisitor ogv = 0;
	List circuit = 0;
	List output_gates = 0;

	AssertTrue(buf != 0)
	AssertTrue(fp != 0)
	oe->read(fp,buf,&lbuf);
	oe->close(fp);

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);

	// zero is written but never read
	circuit = cp->parseSource(buf,lbuf);
	AssertTrue(circuit != 0)

	ogv = OutputGateVisitor_New(oe);
	AssertTrue(ogv != 0)

	output_gates = ogv->visit(circuit);
	AssertTrue(output_gates != 0)
	//AssertTrue(output_gates->size() == 299)
	{
		uint i = 0;
		byte b[4096] = {0};
		osal_sprintf(b,"\n*** Number of output gates: %lu ***\n",output_gates->size());
		oe->p(b);
		for(i = 0; i < output_gates->size();++i) {
			osal_sprintf(b+(6*i),"%05lu\n",(ull)output_gates->get_element(i));
		}
		oe->p(b);
	}
	//AssertTrue( ((ull)output_gates->get_element(0)) == 0 )

	test_end:
	return ok;


}


Test tests[] = {
		{"create ogvisitor",create_ogv},
		{"test with one and gate",test_run_one_and_gate},
		{"test with one xor gate", test_run_one_xor_gate},
		{"test with aes circuit", test_with_aes},

};

TEST_MAIN

