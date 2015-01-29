/*
 * linear_proof.c
 *
 *  Created on: Jan 9, 2015
 *      Author: rwl
 */


#include <linear_proof.h>
#include <coov3.h>

#include <rnd.h>

typedef struct _ptb_impl_ {
	byte * permaj;
	uint lpermaj;
	byte and_challenge;
	uint and_count;
	uint circuit_size;
	List result;
	OE oe ;
} * Ptb;

COO_DCL(CircuitVisitor, void *, ptb_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ptb_visit, List circuit;,circuit) {
	uint i = 0;
	Ptb impl = this->impl;
	uint expected_and_count_max = 0;
	expected_and_count_max = impl->lpermaj*8 + (impl->and_challenge == 0 ? 2 : 1);
	expected_and_count_max /= (impl->and_challenge == 0 ? 3 : 2);

	if (impl->result) {
		SingleLinkedList_destroy(&impl->result);
	}
	impl->result = SingleLinkedList_new(impl->oe);
	impl->and_count = 0;
	impl->circuit_size = circuit->size();

	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_XOR) { this->visitXor(g); }
		if (g->type == G_AND) { this->visitAnd(g); }

		if (impl->and_count > expected_and_count_max) {
			impl->oe->syslog(OSAL_LOGLEVEL_WARN,"More and gates than permutations/majority tests.");
		}
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
	impl->result->add_element(pt);
	return 0;
}

static const uint majs[3][3] = {{0,1,2},{0,2,1},{2,1,0}};
static void apply_majority(OE oe, byte * m, uint lm, uint * indx, uint majidx) {
	uint byte_idx = 0;
	uint bit_idx = 0;
	byte bit1 = 0, bit2 = 0;
	uint res[3] = {0};
	uint index = 0;

	byte_idx = (2*majidx)/8;
	bit_idx = (2*majidx+7)/8-byte_idx;

	bit1 = (((0x01 << bit_idx) & m[byte_idx]) != 0);
	if (bit_idx > 6) {
		byte_idx += 1;
		if (byte_idx >= lm) {
			oe->syslog(OSAL_LOGLEVEL_WARN, "Asking for majority bits at index out of bounds.");
			return;
		}
		bit_idx = 0;
	}
	bit2 = (((0x01 << bit_idx) & m[byte_idx]) != 0);
	index = bit2;
	index <<= 1;
	index += bit1;
	if (index > 2) {
		oe->syslog(OSAL_LOGLEVEL_WARN,"Asking for majority permutation at index out of bounds.");
		return;
	}
	res[majs[index][0]] = indx[0];
	res[majs[index][1]] = indx[1];
	res[majs[index][2]] = indx[2];
	indx[0] = res[0];
	indx[1] = res[1];
	indx[2] = res[2];
}

static const uint perms[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
static void apply_permutation(OE oe, byte * p, uint lp, uint * indx, uint permidx) {
	uint byte_idx = 0;
	uint bit_idx = 0;
	byte bit1=0, bit2=0, bit3=0;
	uint res[3] = {0};

	uint perm_to_use = 0;

	byte_idx = (3*permidx)/8;
	bit_idx = (3*permidx)-byte_idx*8;

	bit1 = (((0x01 << bit_idx) & p[byte_idx]) != 0);
	if (bit_idx > 6) {
		byte_idx += 1;
		if (byte_idx > lp) {
			oe->syslog(OSAL_LOGLEVEL_WARN, "Extracting bit2 we ran index out of bound.");
			return;
		}
		bit_idx=0;
	} else {
		bit_idx += 1;
	}
	bit2 = (((0x01 << bit_idx) & p[byte_idx]) != 0);
	if (bit_idx > 6) {
		byte_idx += 1;
		if (byte_idx > lp) {
			oe->syslog(OSAL_LOGLEVEL_WARN, "Extracting bit3 we ran index out of bound.");
			return;
		}
		bit_idx=0;
	} else {
		bit_idx += 1;
	}
	bit3 = (((0x01 << bit_idx) & p[byte_idx]) != 0);

	perm_to_use = bit3;perm_to_use <<= 1;
	perm_to_use += bit2;perm_to_use <<= 1;
	perm_to_use += bit1;

	if (perm_to_use > 5) {
		oe->syslog(OSAL_LOGLEVEL_WARN,"Permutation to use is out of bound.");
		return;
	}

	res[perms[perm_to_use][0]] = indx[0];
	res[perms[perm_to_use][1]] = indx[1];
	res[perms[perm_to_use][2]] = indx[2];

	indx[0] = res[0];
	indx[1] = res[1];
	indx[2] = res[2];
}

COO_DCL(CircuitVisitor, void *, ptb_visitAnd, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, ptb_visitAnd, Gate and;, and) {
	Ptb impl = this->impl;
	ProofTask pt = impl->oe->getmem(sizeof(*pt));
	uint indx[3] = {0};

	indx[0] = 0;
	indx[1] = 1;
	indx[2] = 2;

	if (impl->and_challenge == 0) {
		// random permutations (three eq tests)
		ProofTask eq1 = 0, eq2 = 0, eq3 = 0;
		eq1 = impl->oe->getmem(sizeof(*eq1));
		eq2 = impl->oe->getmem(sizeof(*eq2));
		eq3 = impl->oe->getmem(sizeof(*eq3));

		apply_permutation(impl->oe,impl->permaj,impl->lpermaj,indx,impl->and_count);

		eq1->indicies[0] = and->dst;
		eq1->indicies[1] = impl->and_count*3 + impl->circuit_size*3+indx[0];

		eq2->indicies[0] = and->op1;
		eq2->indicies[1] = impl->and_count*3 + impl->circuit_size*3+indx[1];

		eq3->indicies[0] = and->op2;
		eq3->indicies[1] = impl->and_count*3 + impl->circuit_size*3+indx[2];

		impl->result->add_element(eq1);
		impl->result->add_element(eq2);
		impl->result->add_element(eq3);
	}

	if (impl->and_challenge == 1) {
		ProofTask eq1 = 0, eq2 = 0;
		eq1 = impl->oe->getmem(sizeof(*eq1));
		eq2 = impl->oe->getmem(sizeof(*eq2));

		apply_majority(impl->oe, impl->permaj,impl->lpermaj, indx, impl->and_count);

		eq1->indicies[0] = and->op1;
		eq1->indicies[1] = impl->and_count*2 + impl->circuit_size*3 + indx[0];

		eq2->indicies[0] = and->op2;
		eq2->indicies[1] = impl->and_count*2 + impl->circuit_size*3 + indx[1];

		impl->result->add_element(eq1);
		impl->result->add_element(eq2);
	}

	if (impl->and_challenge > 1) {
		impl->oe->syslog(OSAL_LOGLEVEL_WARN,"and_challenge is Out of bounds");
	}

	impl->and_count += 1;
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
	CircuitVisitor c = 0;
	Ptb ptb = 0;
	OE oe = 0;

	if (!cv) return;
	c = *cv;

	if (!c) return;
	ptb = c->impl;

	if (!ptb) return;
	oe = ptb->oe;

	COO_DETACH(c,visit);
	COO_DETACH(c,visitAnd);
	COO_DETACH(c,visitXor);

	oe->putmem(c);
	oe->putmem(ptb);

	*cv = 0;
}

void ProofTasks_Destroy(OE oe, List * pt) 	{
	List proofTasks = 0;
	uint i = 0;

	if (!pt) return;
	proofTasks = *pt;

	if (!proofTasks) return;

	for(i = 0;i < proofTasks->size();++i) {
		ProofTask p = proofTasks->get_element(i);
		oe->putmem(p);
	}
	SingleLinkedList_destroy(&proofTasks);
}

typedef struct _gpam_impl_ {
	OE oe;
	Rnd rnd;
	uint no_ands;
	GPam result;
	uint and_count;
	byte * evaled_circuit;
} * GPamImpl;;

COO_DCL(CircuitVisitor, void *, gpam_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, gpam_visit, List circuit;, circuit) {
	GPamImpl impl = (GPamImpl)this->impl;
	GPam r = 0;
	impl->result = impl->oe->getmem(sizeof(*(impl->result)));
	uint no_ands = 0;
	uint i = 0;

	// compute result size
	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_AND) ++no_ands;
	}

	impl->result->no_ands = impl->no_ands = no_ands;
	impl->result->permutations = impl->oe->getmem( (no_ands*3+7)/8);
	impl->result->majority = impl->oe->getmem( (no_ands*2 + 7)/8);
	impl->result->aux = impl->oe->getmem( (no_ands*3+7)/8);


	// visit circuit
	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_XOR) this->visitXor(g);
		if (g->type == G_AND) this->visitAnd(g);
	}

	// release ownership
	r = impl->result;
	impl->result = 0;

	return r;
}

void GPam_Destroy(OE oe, GPam * gpam) {
	GPam p = 0;
	if (!gpam ) return;

	p=*gpam;
	if (p) {
		oe->putmem(p->aux);
		oe->putmem(p->majority);
		oe->putmem(p->permutations);
		oe->putmem(p);
	}
	*gpam = 0;

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



COO_DCL(CircuitVisitor, void *, gpam_visitAnd, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, gpam_visitAnd, Gate and;, and) {
	GPamImpl impl = this->impl;
	byte r = 0;
	uint byte_idx = 0;
	uint bit_idx = 0;
	byte bit1 = 0,bit2 = 0,bit3 = 0;
	byte bits[3] = {0};
	const uint * pi = 0;

	// get uniform random number in [0,..,255]
	impl->rnd->rand(&r,1);

	// TODO(rwz): Reduce random byte modulo 6 gives a small bias to 1,2,3 as 42*6 is 252
	// therefore 253, 254 and 255 modulo six gives an extra candidate for 1,2,3.
	//
	// This can be improved.
	r = r % 6;

	// a random permutation of three elements
	pi = (uint *)perms[r];

	// read bits
	byte_idx = (and->dst)/8;
	bit_idx = and->dst-byte_idx*8;
	bit1 = ( impl->evaled_circuit[byte_idx] & (0x01 << bit_idx) ) != 0;

	byte_idx = (and->op1)/8;
	bit_idx = and->op1 - byte_idx*8;
	bit2 = ( impl->evaled_circuit[byte_idx] & (0x01 << bit_idx) ) != 0;

	byte_idx = (and->op2)/8;
	bit_idx = and->op2 - byte_idx*8;
	bit3 = ( impl->evaled_circuit[byte_idx] & (0x01 << bit_idx) ) != 0;

	set_bit(impl->result->permutations,impl->and_count*3,r & 0x01);
	set_bit(impl->result->permutations,impl->and_count*3+1,(r & 0x02) != 0);
	set_bit(impl->result->permutations,impl->and_count*3+2,(r & 0x04) != 0);

	// apply permutation to bits
	// bits = pi(dst,op1,op2) => (op1,dst,op2)
	bits[pi[0]] = bit1;
	bits[pi[1]] = bit2;
	bits[pi[2]] = bit3;
	bit1 = bits[0]; // op1
	bit2 = bits[1]; // dst
	bit3 = bits[2]; // op2

	// add actual bits to the {aux} data array.
	set_bit(impl->result->aux,impl->and_count*3,bit1);
	set_bit(impl->result->aux,impl->and_count*3+1,bit2);
	set_bit(impl->result->aux,impl->and_count*3+2,bit3);

	// -- setup majority permutation assuming bit1,2,3 are an and gate. --
	// so only patterns 111,100,010,001,000 will appear.
	// 0   0   0
	//bit1bit2bit3
	if (bit1 == 0 && bit2 == 0 && bit3 == 0) {
		set_bit(impl->result->majority,impl->and_count*2,0);
		set_bit(impl->result->majority,impl->and_count*2+1,0);
	}
    // dstop1op2
	// 1   1   1
	// bit1bit2bit3
	if (bit2 == 1 && bit1 == 1 && bit3 == 1) { // both input are one
		set_bit(impl->result->majority,impl->and_count*2,0);
		set_bit(impl->result->majority,impl->and_count*2+1,0);
	}

	// 0 0 1
	if (bit1 == 0 && bit2 == 0 && bit3 == 1) {
		set_bit(impl->result->majority,impl->and_count*2,0);
		set_bit(impl->result->majority,impl->and_count*2+1,0);
	}

	// dst op1 op2
	// 0   1   0
	// bit1bit2bit3
	if (bit1 == 0 && bit2 == 1 && bit3 == 0) {
		set_bit(impl->result->majority,impl->and_count*2,1);
		set_bit(impl->result->majority,impl->and_count*2+1,0);
	}


	// 1 0 0
	if (bit1 == 1 && bit2 == 0 && bit3 == 0) {
		set_bit(impl->result->majority,impl->and_count*2,0);
		set_bit(impl->result->majority,impl->and_count*2+1,1);
	}

	impl->and_count += 1;
	return 0;
}

COO_DCL(CircuitVisitor, void *, gpam_visitXor, Gate xor);
COO_DEF_RET_ARGS(CircuitVisitor, void *, gpam_visitXor, Gate xor;, xor) {
return 0;
}


void ProofTask_print(OE oe, ProofTask pt) {
	byte b[64] = {0};
	osal_sprintf(b, "ProofTask[{%u,%u,%u},%u]",
			pt->indicies[0],
			pt->indicies[1],
			pt->indicies[2],
			pt->value);
	oe->syslog(OSAL_LOGLEVEL_DEBUG,b);
}

CircuitVisitor GeneratePermuationsAndMajorities_New(OE oe, Rnd rnd, byte * eval_circuit) {
	CircuitVisitor iface = (CircuitVisitor)oe->getmem(sizeof(*iface));

	if (!iface) return 0;
	GPamImpl impl = (GPamImpl)oe->getmem(sizeof(*impl));

	if (!impl) goto error;

	impl->oe = oe;
	impl->rnd = rnd;
	impl->evaled_circuit = eval_circuit;
	iface->impl = impl;

	COO_ATTACH_FN(CircuitVisitor,iface,visit,gpam_visit);
	COO_ATTACH_FN(CircuitVisitor,iface,visitAnd, gpam_visitAnd);
	COO_ATTACH_FN(CircuitVisitor,iface,visitXor, gpam_visitXor);

	return iface;
	error:
	GeneratePermutationsAndMajorities_Destroy(&iface);
	return 0;
}

void GeneratePermutationsAndMajorities_Destroy(CircuitVisitor * cv) {
	CircuitVisitor c = 0;
	GPamImpl i = 0;
	OE oe = 0;

	if (!cv) return;

	c = *cv;
	i = c->impl;

	if (!i) return;
	oe = i->oe;

	*cv = 0;
	COO_DETACH(c,visit);
	COO_DETACH(c,visitAnd);
	COO_DETACH(c,visitXor);

	oe->putmem(c);
	oe->putmem(i);
}
