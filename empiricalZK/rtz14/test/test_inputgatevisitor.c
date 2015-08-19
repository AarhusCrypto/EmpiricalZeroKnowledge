/*
 * test_inputgatevisitor.c
 *
 *  Created on: Jan 8, 2015
 *      Author: rwl
 */
#include "test.h"
#include <osal.h>
#include <circuitparser.h>
#include <circuit_analyser.h>
#include <map.h>
#include <datetime.h>
#include <hashmap.h>

static int test_create_input_gate_create(OE oe) {
	CircuitVisitor cv = InputGateVisitor_New(oe);
	_Bool ok= (cv != 0);

	InputGateVisitor_Destroy(&cv);
	return ok;
}


static int test_find_one_input_for_xor(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	List address_of_input_gates = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);
	circuit = cp->parseSource((byte*)"XOR(0,1,1)",11);

	cv = InputGateVisitor_New(oe);
	input_gates = cv->visit(circuit);

	AssertTrue(input_gates != 0)
	address_of_input_gates = input_gates->get_keys();

	AssertTrue(((uint)address_of_input_gates->get_element(0) == 1));

	InputGateVisitor_Destroy(&cv);
	SingleLinkedList_destroy(&address_of_input_gates);

test_end:
	return ok;
}

static int test_find_two_input_for_xor(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	List address_of_input_gates = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);
	circuit = cp->parseSource((byte*)"XOR(0,1,0)",11);

	cv = InputGateVisitor_New(oe);
	address_of_input_gates = cv->visit(circuit);


	AssertTrue((uint)address_of_input_gates->get_element(0) == 1)
	AssertTrue((uint)address_of_input_gates->get_element(1) == 0);

test_end:
	return ok;
}

static int test_find_one_input_for_and(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	List address_of_input_gates = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);
	circuit = cp->parseSource((byte*)"AND(0,1,1)",11);

	cv = InputGateVisitor_New(oe);
	input_gates = cv->visit(circuit);

	address_of_input_gates = input_gates->get_keys();

	ok = input_gates != 0 && input_gates->size() == 1 &&
			(uint)address_of_input_gates->get_element(0) == 1;

	return ok;
}

static int test_find_two_input_for_and(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	List address_of_input_gates = 0;
	Map input_gates = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);
	circuit = cp->parseSource((byte*)"AND(0,1,0)",11);

	cv = InputGateVisitor_New(oe);
	input_gates = cv->visit(circuit);

	AssertTrue(input_gates != 0)
	address_of_input_gates = input_gates->get_keys();

	AssertTrue(	(uint)address_of_input_gates->get_element(0) == 1  );
	AssertTrue( (uint)address_of_input_gates->get_element(1) == 0  );

	HashMap_destroy(&input_gates);
	Circuit_Destroy(oe,&circuit);
	SingleLinkedList_destroy(&address_of_input_gates);
	InputGateVisitor_Destroy(&cv);
test_end:
	return ok;
}

static int test_large_file(OE oe) {
	_Bool ok = 1;
	uint lbuffer = 1060365;
	uint ands = 0, xors = 0, nums = 0, tokens = 0;
	Tokenizer tk = 0;CircuitParser cp = 0;
	List ast = 0;
	byte * buffer = oe->getmem(lbuffer);
	uint fp = 0, i = 0;
	CircuitVisitor cv = 0; 
	Map input_gates = 0;
	List aoig = 0;
	DateTime clock = 0;
	ull start = 0;
	oe->open("file ../test/AES",&fp);
	cv = InputGateVisitor_New(oe);
	clock = DateTime_New(oe);
	start = clock->getMicroTime();
	oe->read(fp,buffer,&lbuffer);
	oe->close(fp);
	DBG_P(oe,"reading file took %u microseconds",clock->getMicroTime()-start);
	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe,tk);
	start = clock->getMicroTime();
	ast = cp->parseSource(buffer,lbuffer);
	DBG_P(oe,"parsing circuit took %u microseconds.",clock->getMicroTime()-start);
	oe->putmem(buffer);


	start = clock->getMicroTime();
	input_gates = cv->visit(ast);
	AssertTrue(input_gates != 0)

	aoig = input_gates->get_keys();
	if (aoig) {
		DBG_P(oe,"#Input: %u analysis took %u microseconds",aoig->size(),clock->getMicroTime()-start);
	}

	AssertTrue( aoig != 0 );
	test_end:
	return ok;
}

Test tests[] = {
		{"Create InputGateVisitor", test_create_input_gate_create},
		{"test_find_one_input_for_xor", test_find_one_input_for_xor},
		{"test_find_two_input_for_xor", test_find_two_input_for_xor},
		{"test_find_one_input_for_and", test_find_one_input_for_and},
		{"test_find_two_input_for_and", test_find_two_input_for_and},
		{"test_large_file", test_large_file}

};


TEST_MAIN
