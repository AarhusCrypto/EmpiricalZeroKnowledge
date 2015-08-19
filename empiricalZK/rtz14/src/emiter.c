/*
 * Emitter.c
 *
 *  Created on: Jan 8, 2015
 *      Author: rwl
 */
#include <emiter.h>
#include <coov4.h>

typedef struct _evaluation_string_emitted_ {
  OE oe;
  byte * bit_string;
  byte * input;
  Map input_gates;
  uint no_and_gates;
  uint next_input;
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

/*
 * Implementation note:
 *
 * The ESEVisitor takes input bits in the order of appearance of the free address
 * in the circuit (which is a sequence of gates, read list).
 *
 */
static byte esev_next_input_bit(ESEVisitor e) {
	byte r = get_bit(e->input,e->next_input);
	e->next_input += 1;
	return r;
}

COO_DEF(CircuitVisitor, void *, ese_visit, List circuit) {
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



	lbit_string = circuit->size()+e->input_gates->size();
	e->no_and_gates = 0;

	e->bit_string = e->oe->getmem((lbit_string+7)/8); // ceil(lbit_string/8)


	for(i = 0;i <  circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_XOR) this->visitXor(g);
		if (g->type == G_AND) this->visitAnd(g);
	}

	er->emitted_string = e->bit_string;
	er->lemitted_string = (lbit_string+7)/8;

	return er;
}}

COO_DEF(CircuitVisitor, void *, ese_visit_and, Gate and) {
	ESEVisitor e = (ESEVisitor)this->impl;
	OE oe = e->oe;
	e->no_and_gates += 1;
	byte op1 = 0, op2 = 0, res = 0;


	if (e->input_gates->contains((void*)(ull)and->op1) == True) {
		op1 = esev_next_input_bit(e);
		set_bit(e->bit_string,and->op1,op1);
		e->input_gates->rem((void*)(ull)and->op1);
	} else {
		op1 = get_bit(e->bit_string,and->op1);
	}

	if (e->input_gates->contains((void*)(ull)and->op2) == True) {
		op2 = esev_next_input_bit(e);
		set_bit(e->bit_string,and->op2,op2);
		e->input_gates->rem((void*)(ull)and->op2);
	} else {
		op2 = get_bit(e->bit_string, and->op2);
	}

	set_bit(e->bit_string,and->dst,op1 & op2);
	e->input_gates->rem((void*)(ull)and->dst);



	return 0;
}}

COO_DEF(CircuitVisitor, void *, ese_visit_xor, Gate xor) {
	ESEVisitor e = (ESEVisitor)this->impl;
	OE oe = e->oe;
	e->no_and_gates += 1;
	byte op1 = 0, op2 = 0, res = 0;

	if (e->input_gates->contains((void*)(ull)xor->op1) == True) {
		op1 = esev_next_input_bit(e);
		set_bit(e->bit_string,xor->op1,op1);
		e->input_gates->rem( (void*)(ull)xor->op1);
	} else {
		op1 = get_bit(e->bit_string,xor->op1);
	}

	if (e->input_gates->contains((void*)(ull)xor->op2) == True) {
		op2 = esev_next_input_bit(e);
		set_bit(e->bit_string,xor->op2,op2);
		e->input_gates->rem((void*)(ull)xor->op2);
	} else {
		op2 = get_bit(e->bit_string, xor->op2);
	}

	set_bit(e->bit_string,xor->dst,op1 ^ op2);
	e->input_gates->rem((void*)(ull)xor->dst);

	return 0;
}}


CircuitVisitor EvaluationStringEmitter_New(OE oe, Map input_gates, byte * input) {
	CircuitVisitor cv = 0;
	ESEVisitor esev = 0;

	if (!oe) return 0;
	if (!input_gates) {
		oe->syslog(OSAL_LOGLEVEL_WARN,"input_gates is not set cannot create EvaluationStringEmitter.");
		return 0;
	}
	if (!input) {
		oe->syslog(OSAL_LOGLEVEL_WARN,"input parameters in not set cannot create EvaluationStringEmitter.");
		return 0;
	}

	cv = oe->getmem(sizeof(*cv));
	if (!cv) return 0;

	esev = oe->getmem(sizeof(*esev));
	if (!esev) goto error;

	esev->oe = oe;
	esev->input_gates = input_gates;
	esev->bit_string = 0;
	esev->input = input;
	esev->next_input = 0;
	cv->impl = esev;

	cv->visit = COO_attach(cv, CircuitVisitor_ese_visit);
	cv->visitAnd = COO_attach(cv, CircuitVisitor_ese_visit_and);
	cv->visitXor = COO_attach(cv, CircuitVisitor_ese_visit_xor);

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

	COO_detach(c->visit);
	COO_detach(c->visitAnd);
	COO_detach(c->visitXor);

	oe->putmem(c);
	oe->putmem(esev);
}


void EmiterResult_Destroy(OE oe, EmiterResult * er) {
	EmiterResult e = 0;
	if (!er) return;
	e = *er;
	*er = 0; // detach before freeing
	oe->putmem(e->emitted_string);
	oe->putmem(e);
}
