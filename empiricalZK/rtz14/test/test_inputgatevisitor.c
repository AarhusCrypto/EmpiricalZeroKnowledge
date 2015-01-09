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

	address_of_input_gates = input_gates->get_keys();

	ok = input_gates != 0 && input_gates->size() == 1 && (uint)address_of_input_gates->get_element(0) == 1;



	return ok;
}

static int test_find_two_input_for_xor(OE oe) {
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
	circuit = cp->parseSource((byte*)"XOR(0,1,0)",11);

	cv = InputGateVisitor_New(oe);
	input_gates = cv->visit(circuit);

	address_of_input_gates = input_gates->get_keys();

	ok = input_gates != 0 && input_gates->size() == 2 &&
			(uint)address_of_input_gates->get_element(0) == 1 &&
			(uint)address_of_input_gates->get_element(1) == 0;

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
	Map input_gates = 0;
	List address_of_input_gates = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe, tk);
	circuit = cp->parseSource((byte*)"AND(0,1,0)",11);

	cv = InputGateVisitor_New(oe);
	input_gates = cv->visit(circuit);

	address_of_input_gates = input_gates->get_keys();

	g = input_gates->get(address_of_input_gates->get_element(0));


	ok = g->type == G_AND && input_gates != 0 && input_gates->size() == 2 &&
			(uint)address_of_input_gates->get_element(0) == 1 &&
			(uint)address_of_input_gates->get_element(1) == 0;

	return ok;
}

static int test_large_file(OE oe) {
	uint lbuffer = 1060365;
	uint ands = 0, xors = 0, nums = 0, tokens = 0;
	Tokenizer tk = 0;CircuitParser cp = 0;
	List ast = 0;
	byte * buffer = oe->getmem(lbuffer);
	uint fp = oe->open("file ../test/AES"), i = 0;
	CircuitVisitor cv = InputGateVisitor_New(oe);
	Map input_gates = 0;
	DateTime clock = 0;
	ull start = 0;
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
	if (input_gates) {
		DBG_P(oe,"#Input: %u analysis took %u microseconds",input_gates->size(),clock->getMicroTime()-start);
	}

	return input_gates != 0;
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
