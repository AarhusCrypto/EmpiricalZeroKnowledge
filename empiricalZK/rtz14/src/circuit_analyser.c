/*
 * circuit_analyser.c



 *
 *  Created on: Jan 7, 2015
 *      Author: rwl
 */
#include <coov3.h>
#include <osal.h>
#include <circuitparser.h>
#include <circuit_analyser.h>
#include <map.h>
#include <hashmap.h>

typedef struct _poc_Visitor_ {
	OE oe;
	uint address_of_one;
} * PocVisitor;

COO_DCL(CircuitVisitor, void *, poc_visitAnd, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, poc_visitAnd, Gate and;,and) {
	PocVisitor pcv = (PocVisitor)this->impl;

	if (and->op2 == -1) {
		and->op2 = pcv->address_of_one;
	}

	if (and->op1 == -1) {
		and->op1 = pcv->address_of_one;
	}

	return 0;
}

COO_DCL(CircuitVisitor, void *, poc_visitXor, Gate xor);
COO_DEF_RET_ARGS(CircuitVisitor, void *, poc_visitXor, Gate xor;,xor) {

	PocVisitor pcv = (PocVisitor)this->impl;

	if (xor->op2 == -1) {
		xor->op2 = pcv->address_of_one;
	}

	if (xor->op1 == -1) {
		xor->op1 = pcv->address_of_one;
	}

	return 0;
}


COO_DCL(CircuitVisitor, void *, poc_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, poc_visit, List circuit;, circuit) {
	uint i = 0;
	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_AND) this->visitAnd(g);
		if (g->type == G_XOR) this->visitXor(g);
	}
	return 0;
}


CircuitVisitor PatchOneConstants_New(OE oe, uint address_of_one) {
	CircuitVisitor cv = (CircuitVisitor)oe->getmem(sizeof(*cv));
	PocVisitor pcv = 0;

	if (!cv) return 0;

	pcv = (PocVisitor)oe->getmem(sizeof(*pcv));
	if (!pcv) goto error;

	COO_ATTACH_FN(CircuitVisitor,cv,visitAnd,poc_visitAnd);
	COO_ATTACH_FN(CircuitVisitor,cv,visitXor,poc_visitXor);
	COO_ATTACH_FN(CircuitVisitor,cv,visit,poc_visit);

	cv->impl = pcv;
	pcv->oe = oe;
	pcv->address_of_one = address_of_one;

	return cv;
	error:
	PatchOneConstants_Destroy(&cv);
	return 0;
}

void PatchOneConstants_Destroy(CircuitVisitor * pcv_) {
	CircuitVisitor cv = 0;
	PocVisitor pcv = 0;
	OE oe = 0;

	if (!pcv_) return;

	cv = *pcv_;
	pcv = (PocVisitor)cv->impl;

	oe = pcv->oe;

	COO_DETACH(cv,visitAnd);
	COO_DETACH(cv, visitXor);
	COO_DETACH(cv, visit);

	oe->putmem(cv);
	oe->putmem(pcv);
}


typedef struct _input_gate_visitor_ {
	OE oe;
	Map write_locations;
	Map input_locations;
} * IGVisitor;

static uint locate_hash(void * a_) {
	uint a = (uint)a_;
	return (a*101 + 65535) % 65537;
}

static int locate_cmp(void *a_, void *b_) {
	uint a = (uint)a_;
	uint b = (uint)b_;
	if (a > b) return 1;
	if (a < b) return -1;
	return 0;
}


COO_DCL(CircuitVisitor, void *, igv_visit_and, Gate and);
COO_DEF_RET_ARGS(CircuitVisitor, void *, igv_visit_and, Gate and;, and) {
	IGVisitor igv = this->impl;
	ull v = and->op1;

	if (igv->write_locations->contains((void*)v) == False) {
		if (igv->input_locations->contains((void*)v) == False) {
			igv->input_locations->put((void*)v,and);
		}
	}

	v= and->op2;
	if (igv->write_locations->contains((void*)v) == False) {
		if (igv->input_locations->contains((void*)v) == False) {
			igv->input_locations->put((void*)v,and);
		}
	}

	v = and->dst;
	igv->write_locations->put((void*)v,and);

	return 0;
}


COO_DCL(CircuitVisitor, void *, igv_visit_xor, Gate xor);
COO_DEF_RET_ARGS(CircuitVisitor, void *, igv_visit_xor, Gate xor;, xor) {
	IGVisitor igv = this->impl;
	ull v = 0;

	v = xor->op1;
	if (igv->write_locations->contains((void*)v) == False) {
		if (igv->input_locations->contains((void*)v) == False) {
			igv->input_locations->put((void*)v,xor);
		}
	}

	v = xor->op2;
	if (igv->write_locations->contains((void*)v) == False) {
		if (igv->input_locations->contains((void*)v) == False) {
			igv->input_locations->put((void*)v,xor);
		}
	}

	v = xor->dst;
	igv->write_locations->put((void*)v,xor);

	return 0;
}


COO_DCL(CircuitVisitor, void *, igv_visit, List circuit);
COO_DEF_RET_ARGS(CircuitVisitor, void *, igv_visit, List circuit;, circuit) {
	uint i = 0;
	IGVisitor igv = this->impl;
	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		List input_g = 0;
		switch(g->type) {
		case G_AND: {
			this->visitAnd(g);
			break;
		case G_XOR: {
			this->visitXor(g);
			break;
		}
		default:
			break;
		}
		}
	}
	return igv->input_locations;
}

CircuitVisitor InputGateVisitor_New(OE oe) {
	CircuitVisitor cv = (CircuitVisitor)oe->getmem(sizeof(*cv));
	IGVisitor igv = 0;

	if (!cv) return 0;

	igv = (IGVisitor)oe->getmem(sizeof(*igv));
	if (!igv) goto error;

	igv->oe = oe;
	igv->write_locations = HashMap_new(oe, locate_hash,locate_cmp,64);
	igv->input_locations = HashMap_new(oe, locate_hash,locate_cmp,64);

	COO_ATTACH_FN(CircuitVisitor, cv, visitAnd, igv_visit_and);
	COO_ATTACH_FN(CircuitVisitor, cv, visitXor, igv_visit_xor);
	COO_ATTACH_FN(CircuitVisitor, cv, visit, igv_visit);

	cv->impl = igv;

	return cv;
	error:
	InputGateVisitor_Destroy(&cv);
	return 0;
}

void InputGateVisitor_Destroy(CircuitVisitor * cv_) {
	CircuitVisitor cv = 0;
	IGVisitor igv = 0;
	OE oe = 0;

	if (!cv_) return;

	cv = *cv_;
	igv = cv->impl;

	oe = igv->oe;

	HashMap_destroy(&igv->input_locations);
	HashMap_destroy(&igv->write_locations);
	oe->putmem(cv);
	oe->putmem(igv);
}
