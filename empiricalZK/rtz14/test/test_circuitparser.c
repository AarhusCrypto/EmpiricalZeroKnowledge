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
	_Bool res = 0;
	Tokenizer tk = FunCallTokenizer_New(oe);
	CircuitParser cp = CircuitParser_New(oe,tk
			);
	cp->parseSource("XOR 1x2c3",0);
	res = cp != 0;
	CircuitParser_Destroy(&cp);
	return res && cp == 0;
}

static int parse_correct_xor(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe);
	CircuitParser cp = CircuitParser_New(oe,tk);
	List gates = 0;
	gates = cp->parseSource("XOR(1,2,3,0)",12);
	CircuitParser_Destroy(&cp);
	return gates->size() == 1;
}

static int parse_correct_and(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe);
	CircuitParser cp = CircuitParser_New(oe,tk);
	List gates = 0;
	gates = cp->parseSource("AND(1,2,3,0)",12);
	CircuitParser_Destroy(&cp);
	return gates->size() == 1;

}

static int parse_correct_inv(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe);
	CircuitParser cp = CircuitParser_New(oe,tk);
	List gates = 0;
	gates = cp->parseSource("INV(30539,30384,2);",12);
	CircuitParser_Destroy(&cp);
	return gates->size() == 1;
}

static int parse_file(OE oe) {
	uint lbuffer = 1060365;
	uint ands = 0, xors = 0, nums = 0, tokens = 0;
	Tokenizer tk = 0;CircuitParser cp = 0;
	List ast = 0;
	byte * buffer = oe->getmem(lbuffer);
	uint fp = 0, i = 0;
	oe->open("file ../test/AES",&fp);
	oe->read(fp,buffer,&lbuffer);
	oe->close(fp);
	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe,tk);
	ast = cp->parseSource(buffer,lbuffer);
	oe->putmem(buffer);

	for(i = 0; i < ast->size();++i) {
		Gate g = (Gate)ast->get_element(i);
		if (g) {
 			if (g->type == G_AND) ands++;
 			else
 				if (g->type == G_XOR) xors++;
 				else {
 					DBG_P(oe,"Wierd gate","");
 				}
		}
	}

	DBG_P(oe,"{xors:%u, ands:%u, size: %u, sum: %u}",xors,ands,ast->size(),xors+ands);

	return ands == 6800 && xors == 26139 && ast->size() == ands+xors;
}

Test tests[] = {
		{"Creation of function based circuit parser", create_fun_circuit_parser},
		{"Parse correct XOR",parse_correct_xor},
		{"parse correct AND",parse_correct_and},
		{"parse_correct_INV",parse_correct_inv},
		{"parse_large_file",parse_file},
};

TEST_MAIN
