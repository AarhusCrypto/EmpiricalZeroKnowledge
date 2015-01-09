/*
 * test_emitter.c
 *
 *  Created on: Jan 9, 2015
 *      Author: rwl
 */


#include "test.h"
#include <osal.h>
#include <circuit_analyser.h>
#include <circuitparser.h>
#include <map.h>
#include "emitter.h"

static int test_emit_and_gate(OE oe) {
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emitter = 0;
	List circuit = 0;
	Map input_gates = 0;
	byte witness[1] = {3}; // 00000011b
	byte * emitted_string = 0;

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe,tk);
	igv = InputGateVisitor_New(oe);
	circuit = cp->parseSource((byte*)"AND(2,0,1)",11);
	input_gates = igv->visit(circuit);

	emitter = EvaluationStringEmitter_New(oe,input_gates,witness);
	emitted_string = emitter->visit(circuit);

	return emitted_string != 0 && emitted_string[0] == 7; // 00000111b

}

static int test_emitter_create(OE oe) {
	Tokenizer tk = 0;
	CircuitParser cp = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emitter = 0;
	List circuit = 0;
	Map input_gates = 0;
	byte witness[1] = {3}; // 00000011b

	tk = FunCallTokenizer_New(oe);
	cp = CircuitParser_New(oe,tk);
	igv = InputGateVisitor_New(oe);
	circuit = cp->parseSource((byte*)"AND(1,0,2)",11);
	input_gates = igv->visit(circuit);

	emitter = EvaluationStringEmitter_New(oe,input_gates,witness);
	emitter->visit(circuit);

	return emitter != 0;
}

Test tests[] = {
		{"test emitter creation",test_emitter_create},
		{"test emit and gate", test_emit_and_gate},

};

TEST_MAIN
