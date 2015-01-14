/*
 * linear_proof.c
 *
 *  Created on: Jan 9, 2015
 *      Author: rwl
 */


#include <linear_proof.h>
#include <coov3.h>

typedef struct _ptb_impl_ {
	byte * permaj;
	uint lpermaj;
	byte and_challenge;
	uint and_count;
	List result;
	OE oe ;
} * Ptb;

COO_DCL(CircuitVisitor, void *, ptb_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ptb_visit, List circuit;,circuit) {
	uint i = 0;
	Ptb impl = this->impl;

	if (impl->result) {
		SingleLinkedList_destroy(&impl->result);
	}
	impl->result = SingleLinkedList_new(impl->oe);
	impl->and_count = 0;

	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_XOR) { this->visitXor(g); }
		if (g->type == G_AND) { this->visitAnd(g); }
	}
	return impl->result;
}

COO_DCL(CircuitVisitor, void *, ptb_visitXor, Gate xor);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ptb_visitXor, Gate xor;, xor) {
	Ptb impl = this->impl;
	ProofTask pt = impl->oe->getmem(sizeof(*pt));
	pt->indicies[0] = xor->dst;
	pt->indicies[1] = xor->op1;
	pt->indicies[2] = xor->op2;
	pt->value = 0;
	return 0;
}

static inline uint read_bit(byte * bit_str,uint idx) {
	uint byte_index = idx/8;
	uint bit_index_in_byte = idx % 8;
	return bit_str[byte_index] & (0x01 << bit_index_in_byte) != 0;
}


static void apply_permutation(byte * p, byte * cand, uint idx) {

}

COO_DCL(CircuitVisitor, void *, ptb_visitAnd, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ptb_visitAnd, Gate and;, and) {
	Ptb impl = this->impl;
	ProofTask pt = impl->oe->getmem(sizeof(*pt));

	if (impl->and_challenge == 0) {
		// random permutations (three eq tests)
		uint indx[3] = {0};
		ProofTask eq1 = 0, eq2 = 0, eq3 = 0;
		eq1 = impl->oe->getmem(sizeof(*eq1));
		eq2 = impl->oe->getmem(sizeof(*eq2));
		eq3 = impl->oe->getmem(sizeof(*eq3));

		indx[0] = 1;
		indx[1] = 2;
		indx[2] = 3;
		apply_permutation(impl->lpermaj,impl->and_count,indx);

		eq1->indicies[0] = and->dst;
		//eq1->indicies[1] =
	}

	return 0;
}

CircuitVisitor ProofTaskBuilder_New(OE oe, byte and_challenge, byte * permaj, uint lpermaj) {
	CircuitVisitor cv = (CircuitVisitor)oe->getmem(sizeof(*cv));
	Ptb ptb = 0;

	COO_ATTACH_FN(CircuitVisitor, cv, visit, ptb_visit);
	COO_ATTACH_FN(CircuitVisitor, cv, visitXor, ptb_visitXor);
	COO_ATTACH_FN(CircuitVisitor, cv, visitAnd, ptb_visitAnd);

	ptb = (Ptb)oe->getmem(sizeof(*ptb));
	ptb->oe = oe;
	ptb->and_challenge = and_challenge;
	ptb->permaj = permaj;
	ptb->lpermaj = lpermaj;
	cv->impl = ptb;

	return cv;
}

void ProofTaskBuilder_Destroy(CircuitVisitor * cv) {

}
