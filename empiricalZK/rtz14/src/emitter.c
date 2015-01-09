/*
 * emitter.c
 *
 *  Created on: Jan 8, 2015
 *      Author: rwl
 */
#include <emitter.h>
#include <coov3.h>

typedef struct _evaluation_string_emitted_ {
  OE oe;
  byte * bit_string;
  byte * input;
  Map input_gates;
  uint no_and_gates;
} * ESEVisitor;

static inline uint read_bit(byte * bit_str,uint idx) {
	uint byte_index = idx/8;
	uint bit_index_in_byte = idx % 8;
	return bit_str[byte_index] & (0x01 << bit_index_in_byte) != 0;
}

static inline void write_bit(byte * bit_str, uint idx, byte bit) {
	uint byte_index = idx/8;
	uint bit_index_in_byte = idx - 8*byte_index;
	bit = (bit & 0x01) << bit_index_in_byte;
	bit_str[byte_index] |= bit;
}


COO_DCL(CircuitVisitor, void *, ese_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ese_visit, List circuit;,circuit) {
	uint lbit_string = 0;
	ESEVisitor e = (ESEVisitor)this->impl;
	uint i = 0, no_ands = 0;

	if (e->bit_string) {
		e->oe->putmem(e->bit_string);
	}

	for(i = 0;i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_AND)
			++no_ands;
	}


	lbit_string = circuit->size()*3+3*no_ands;
	e->no_and_gates = 0;

	e->bit_string = e->oe->getmem((lbit_string+7)/8); // ceil(lbit_string/8)


	for(i = 0;i <  circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_XOR) this->visitXor(g);
		if (g->type == G_AND) this->visitAnd(g);
	}

	return e->bit_string;
}

COO_DCL(CircuitVisitor, void *, ese_visit_and, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ese_visit_and, Gate and;,and) {
	ESEVisitor e = (ESEVisitor)this->impl;
	OE oe = e->oe;
	e->no_and_gates += 1;
	byte op1 = 0, op2 = 0, res = 0;


	if (e->input_gates->contains(and->op1) == True) {
		// TODO(RWZ): This is not going to work for our 1 constant
		// as it is loaded in the very last address.
		// however we will take care of this explicitly.
		if (and->op1 > e->input_gates->size()) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"Danger: reading input outside of region. The inputs should be ordered such that they are in the first N addresses of the heap.");
			op1 = 0;
		} else {
			op1 = read_bit(e->input, and->op1);
			write_bit(e->bit_string,and->op1,op1);
		}
	} else {
		op1 = read_bit(e->bit_string,and->op1);
	}

	if (e->input_gates->contains(and->op2) == True) {
		if (and->op2 > e->input_gates->size()) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"Danger: reading input outside of region. The inputs should be ordered such that they are in the first N addresses of the heap.");
			op2 = 0;
		} else {
			op2 = read_bit(e->input, and->op2);
			write_bit(e->bit_string,and->op2,op2);
		}
	} else {
		op2 = read_bit(e->bit_string, and->op2);
	}

	write_bit(e->bit_string,and->dst,op1 & op2);



	return 0;
}

COO_DCL(CircuitVisitor, void *, ese_visit_xor, Gate xor);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ese_visit_xor, Gate xor;,xor) {
	ESEVisitor e = (ESEVisitor)this->impl;
	OE oe = e->oe;
	e->no_and_gates += 1;
	byte op1 = 0, op2 = 0, res = 0;

	if (e->input_gates->contains(xor->op1) == True) {
		// TODO(RWZ): This is not going to work for our 1 constant
		// as it is loaded in the very last address.
		// however we will take care of this explicitly.
		if (xor->op1 > e->input_gates->size()) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"Danger: reading input outside of region. The inputs should be ordered such that they are in the first N addresses of the heap.");
			op1 = 0;
		} else {
			op1 = read_bit(e->input, xor->op1);
			write_bit(e->bit_string,xor->op1,op1);
		}
	} else {
		op1 = read_bit(e->bit_string,xor->op1);
	}

	if (e->input_gates->contains(xor->op2) == True) {
		if (xor->op2 > e->input_gates->size()) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"Danger: reading input outside of region. The inputs should be ordered such that they are in the first N addresses of the heap.");
			op2 = 0;
		} else {
			op2 = read_bit(e->input, xor->op2);
			write_bit(e->bit_string,xor->op2,op2);
		}
	} else {
		op2 = read_bit(e->bit_string, xor->op2);
	}

	write_bit(e->bit_string,xor->dst,op1 ^ op2);

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


