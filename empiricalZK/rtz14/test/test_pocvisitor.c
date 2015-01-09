#include "test.h"
#include <osal.h>
#include <circuit_analyser.h>
#include <circuitparser.h>

static int inv_poc_test(OE oe) {
   CircuitParser cp = 0;
   Tokenizer tk = 0;
   CircuitVisitor cv = 0 ;
   List circuit = 0;
   Gate invGate = 0;
   _Bool ok = 0;
   tk = FunCallTokenizer_New(oe);
   cp = CircuitParser_New(oe,tk);
   cv = PatchOneConstants_New(oe,3200);

   circuit = cp->parseSource((byte*)"INV(0,0)",10);
   cv->visit(circuit);

   invGate = circuit->get_element(0);
   FunCallTokenizer_Destroy(&tk);
   CircuitParser_Destroy(&cp);
   PatchOneConstants_Destroy(&cv);
   SingleLinkedList_destroy(&circuit);

   ok = invGate != 0 && invGate->op2 == 3200 && invGate->type == G_XOR;
   oe->putmem(invGate);

   return ok;
}

static int create_poc_test(OE oe) {
	_Bool ok = 0;
	CircuitVisitor cv = PatchOneConstants_New(oe,42);
	ok = (cv != 0);
	PatchOneConstants_Destroy(&cv);
	return ok;
}

static int xor_op1_poc_test(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	if (!tk) return 0;

	cp = CircuitParser_New(oe,tk);
	if (!cp) goto failure;

	cv = PatchOneConstants_New(oe, 42);
	if (!cv) goto failure;

	circuit = cp->parseSource((byte*)"XOR(0,4294967295,2);",20);
	if (circuit == 0 || circuit->size() != 1) goto failure;

	cv->visit(circuit);

	g = circuit->get_element(0);

	ok = g != 0 && g->type == G_XOR && g->op1 == 42 && g->op2 == 2 && g->dst == 0;

	oe->putmem(g);
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);

	return ok;
	failure:
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	return 0;
}


static int xor_op2_poc_test(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	if (!tk) return 0;

	cp = CircuitParser_New(oe,tk);
	if (!cp) goto failure;

	cv = PatchOneConstants_New(oe, 42);
	if (!cv) goto failure;

	circuit = cp->parseSource((byte*)"XOR(0,2,4294967295);",20);
	if (circuit == 0 || circuit->size() != 1) goto failure;

	cv->visit(circuit);

	g = circuit->get_element(0);

	ok = g != 0 && g->type == G_XOR && g->op2 == 42 && g->op1 == 2 && g->dst == 0;

	oe->putmem(g);
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);

	return ok;
	failure:
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	return 0;
}


static int and_op1_poc_test(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	if (!tk) return 0;

	cp = CircuitParser_New(oe,tk);
	if (!cp) goto failure;

	cv = PatchOneConstants_New(oe, 42);
	if (!cv) goto failure;

	circuit = cp->parseSource((byte*)"AND(0,4294967295,2);",20);
	if (circuit == 0 || circuit->size() != 1) goto failure;

	cv->visit(circuit);

	g = circuit->get_element(0);

	ok = g != 0 && g->type == G_AND && g->op1 == 42 && g->op2 == 2 && g->dst == 0;

	oe->putmem(g);
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);

	return ok;
	failure:
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	return 0;
}


static int and_op2_poc_test(OE oe) {
	CircuitVisitor cv = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Gate g = 0;
	_Bool ok = 0;

	tk = FunCallTokenizer_New(oe);
	if (!tk) return 0;

	cp = CircuitParser_New(oe,tk);
	if (!cp) goto failure;

	cv = PatchOneConstants_New(oe, 42);
	if (!cv) goto failure;

	circuit = cp->parseSource((byte*)"XOR(0,2,4294967295);",20);
	if (circuit == 0 || circuit->size() != 1) goto failure;

	cv->visit(circuit);

	g = circuit->get_element(0);

	ok = g != 0 && g->type == G_AND && g->op2 == 42 && g->op1 == 2 && g->dst == 0;

	oe->putmem(g);
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);

	return ok;
	failure:
	PatchOneConstants_Destroy(&cv);
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	return 0;
}


Test tests[] = {
		{"create patch ones visitor", create_poc_test},
		{"inv_into_xor_with_path_4_one",inv_poc_test},
		{"xor_op1_poc_test", xor_op1_poc_test},
		{"xor_op2_poc_test", xor_op2_poc_test},
		{"and_op1_poc_test", and_op1_poc_test},
		{"and_op2_poc_test", and_op1_poc_test}
};

TEST_MAIN
