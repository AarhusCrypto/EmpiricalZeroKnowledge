/*
 * emitter.c
 *
 *  Created on: Jan 8, 2015
 *      Author: rwl
 */
#include <emiter.h>
#include <coov3.h>

typedef struct _evaluation_string_emitted_ {
  OE oe;
  byte * bit_string;
  byte * input;
  Map input_gates;
  uint no_and_gates;
} * ESEVisitor;

static inline byte get_bit(byte * bita, uint idx) {
	uint byte_idx = idx/8;
	uint bit_idx = idx-byte_idx*8;
	byte mask =  0x01 << bit_idx;
	return (bita[byte_idx] & mask) != 0;
}

static inline void set_bit(byte * bita, uint idx, byte bit) {
	uint byte_idx = idx/8;
	uint bit_idx = idx-byte_idx*8;
	byte mask =  0x01 << bit_idx;
	if (bit == 1)
		bita[byte_idx] |= mask;
	if (bit == 0)
		bita[byte_idx] &= ~mask;
}


COO_DCL(CircuitVisitor, void *, ese_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ese_visit, List circuit;,circuit) {
	uint lbit_string = 0;
	ESEVisitor e = (ESEVisitor)this->impl;
	uint i = 0, no_ands = 0;
	EmiterResult er = 0;

	if (e->bit_string) {
		e->oe->putmem(e->bit_string);
	}

	for(i = 0;i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_AND)
			++no_ands;
	}

	er = e->oe->getmem(sizeof(*er));



	lbit_string = circuit->size()*3+3*no_ands;
	e->no_and_gates = 0;

	e->bit_string = e->oe->getmem((lbit_string+7)/8); // ceil(lbit_string/8)


	for(i = 0;i <  circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_XOR) this->visitXor(g);
		if (g->type == G_AND) this->visitAnd(g);
	}

	er->emitted_string = e->bit_string;

	return er;
}

COO_DCL(CircuitVisitor, void *, ese_visit_and, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ese_visit_and, Gate and;,and) {
	ESEVisitor e = (ESEVisitor)this->impl;
	OE oe = e->oe;
	e->no_and_gates += 1;
	byte op1 = 0, op2 = 0, res = 0;


	if (e->input_gates->contains(and->op1) == True) {
		op1 = get_bit(e->input, and->op1);
		set_bit(e->bit_string,and->op1,op1);
		e->input_gates->rem(and->op1);
	} else {
		op1 = get_bit(e->bit_string,and->op1);
	}

	if (e->input_gates->contains(and->op2) == True) {
		op2 = get_bit(e->input, and->op2);
		set_bit(e->bit_string,and->op2,op2);
		e->input_gates->rem(and->op2);
	} else {
		op2 = get_bit(e->bit_string, and->op2);
	}

	set_bit(e->bit_string,and->dst,op1 & op2);
	e->input_gates->rem(and->dst);



	return 0;
}

COO_DCL(CircuitVisitor, void *, ese_visit_xor, Gate xor);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ese_visit_xor, Gate xor;,xor) {
	ESEVisitor e = (ESEVisitor)this->impl;
	OE oe = e->oe;
	e->no_and_gates += 1;
	byte op1 = 0, op2 = 0, res = 0;

	if (e->input_gates->contains(xor->op1) == True) {
		op1 = get_bit(e->input, xor->op1);
		set_bit(e->bit_string,xor->op1,op1);
		e->input_gates->rem(xor->op1);
	} else {
		op1 = get_bit(e->bit_string,xor->op1);
	}

	if (e->input_gates->contains(xor->op2) == True) {
		op2 = get_bit(e->input, xor->op2);
		set_bit(e->bit_string,xor->op2,op2);
		e->input_gates->rem(xor->op2);
	} else {
		op2 = get_bit(e->bit_string, xor->op2);
	}

	set_bit(e->bit_string,xor->dst,op1 ^ op2);
	e->input_gates->rem(xor->dst);

	return 0;
}


CircuitVisitor EvaluationStringEmitter_New(OE oe, Map input_gates, byte * input) {
	CircuitVisitor cv = 0;
	ESEVisitor esev = 0;

	cv = oe->getmem(sizeof(*cv));
	if (!cv) return 0;

	esev = oe->getmem(sizeof(*esev));
	if (!esev) goto error;

	esev->oe = oe;
	esev->input_gates = input_gates;
	esev->bit_string = 0;
	esev->input = input;
	cv->impl = esev;

	COO_ATTACH_FN(CircuitVisitor,cv,visit,ese_visit);
	COO_ATTACH_FN(CircuitVisitor,cv,visitAnd, ese_visit_and);
	COO_ATTACH_FN(CircuitVisitor,cv,visitXor, ese_visit_xor);

	return cv;
	error:
	EvaluationStringEmitter_Destroy(&cv);
	return 0;
}

void EvaluationStringEmitter_Destroy(CircuitVisitor * cv) {
	CircuitVisitor c =0;
	ESEVisitor esev = 0;
	OE oe = 0;
	if (!cv) return;

	c = *cv;
	esev = (ESEVisitor)c->impl;
	oe = esev->oe;

	COO_DETACH(c,visit);
	COO_DETACH(c,visitAnd);
	COO_DETACH(c,visitXor);

	oe->putmem(c);
	oe->putmem(esev);
}


void EmiterResult_Destroy(OE oe, EmiterResult * er) {
	EmiterResult e = 0;
	if (!er) return;
	e = *er;
	*er = 0; // detach before freeing
	oe->putmem(e->emitted_string);
	oe->putmem(e->major);
	oe->putmem(e->perms);
	oe->putmem(e);
}
