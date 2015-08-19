/*
 * circuit_analyser.c



 *
 *  Created on: Jan 7, 2015
 *      Author: rwl
 */
#include <coov4.h>
#include <osal.h>
#include <circuitparser.h>
#include <circuit_analyser.h>
#include <map.h>
#include <hashmap.h>

typedef struct _poc_Visitor_ {
	OE oe;
	uint address_of_one;
} * PocVisitor;

COO_DEF(CircuitVisitor, void *, poc_visitAnd, Gate and)
	PocVisitor pcv = (PocVisitor)this->impl;

	if (and->op2 == -1) {
		and->op2 = pcv->address_of_one;
	}

	if (and->op1 == -1) {
		and->op1 = pcv->address_of_one;
	}

	return 0;
}

COO_DEF(CircuitVisitor, void *, poc_visitXor, Gate xor)
	PocVisitor pcv = (PocVisitor)this->impl;

	if (xor->op2 == -1) {
		xor->op2 = pcv->address_of_one;
	}

	if (xor->op1 == -1) {
		xor->op1 = pcv->address_of_one;
	}

	return 0;
}


COO_DEF(CircuitVisitor, void *, poc_visit, List circuit)
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

	cv->visitAnd = COO_attach(cv,CircuitVisitor_poc_visitAnd);
	cv->visitXor = COO_attach(cv,CircuitVisitor_poc_visitXor);
	cv->visit    = COO_attach(cv,CircuitVisitor_poc_visit);

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

	COO_detach(cv->visitAnd);
	COO_detach(cv->visitXor);
	COO_detach(cv->visit);

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


COO_DEF(CircuitVisitor, void *, igv_visit_and, Gate and)
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


COO_DEF(CircuitVisitor, void *, igv_visit_xor, Gate xor)
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


COO_DEF(CircuitVisitor, void *, igv_visit, List circuit)

	uint i = 0;
	IGVisitor igv = this->impl;
	Map result = 0;

	if (!igv->input_locations) {
		igv->input_locations = HashMap_new(igv->oe,locate_hash, locate_cmp, 64);
	}

	if (!igv->write_locations) {
		igv->write_locations = HashMap_new(igv->oe, locate_hash,locate_cmp,64);
	}

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


	// Return ownership to the caller
	HashMap_destroy(&igv->write_locations);
	result = igv->input_locations;
	igv->input_locations = 0;
	return result;
}

CircuitVisitor InputGateVisitor_New(OE oe) {
	CircuitVisitor cv = (CircuitVisitor)oe->getmem(sizeof(*cv));
	IGVisitor igv = 0;

	if (!cv) return 0;

	igv = (IGVisitor)oe->getmem(sizeof(*igv));
	if (!igv) goto error;

	igv->oe = oe;

	cv->visitAnd = COO_attach(cv, CircuitVisitor_igv_visit_and);
	cv->visitXor = COO_attach(cv, CircuitVisitor_igv_visit_xor);
	cv->visit    = COO_attach(cv, CircuitVisitor_igv_visit);

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


typedef struct _output_gate_visitor_impl_ {
	OE oe ;
	Map gates_read;
} * OGVImpl;


COO_DEF(CircuitVisitor, void *, ogv_visitAnd, Gate and)

	OGVImpl ogv = (OGVImpl)this->impl;

	if (ogv->gates_read->contains((void*)(ull)and->op1) == False) {
		ogv->gates_read->put((void*)(ull)and->op1, and);
	}

	if (!ogv->gates_read->contains((void*)(ull)and->op2) == False) {
		ogv->gates_read->put((void*)(ull)and->op2,and);
	}

	return 0;
}


COO_DEF(CircuitVisitor, void *, ogv_visitXor, Gate xor)

	OGVImpl ogv = (OGVImpl)this->impl;

	if (ogv->gates_read->contains((void*)(ull)xor->op1) == False) {
		ogv->gates_read->put((void*)(ull)xor->op1, xor);
	}

	if (!ogv->gates_read->contains((void*)(ull)xor->op2) == False) {
		ogv->gates_read->put((void*)(ull)xor->op2,xor);
	}

	return 0;
}


COO_DEF(CircuitVisitor, void *, ogv_visit, List circuit)

	OGVImpl ogv = (OGVImpl)this->impl;
	uint i = 0;
	Map h = HashMap_new(ogv->oe, locate_hash, locate_cmp, 1024);
	List res = 0 ;

	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (h->contains( (void*)(ull)g->op1) == True) {
			h->rem( (void*)(ull)g->op1);
		}

		if (h->contains( (void*)(ull)g->op2) == True) {
			h->rem( (void*)(ull)g->op2);
		}

		if (h->contains( (void*)(ull)g->dst) == False) {
			h->put( (void*)(ull)g->dst, g);
		}

	}
	res = h->get_keys();
	HashMap_destroy(&h);
	return res;
}

CircuitVisitor OutputGateVisitor_New(OE oe) {
	CircuitVisitor res = (CircuitVisitor)oe->getmem(sizeof(*oe));
	OGVImpl impl = 0;

	if (!res) return 0;
	impl = (OGVImpl)oe->getmem(sizeof(*impl));

	if (!impl) return 0;

	impl->oe = oe;
	res->impl = impl;

	res->visit = COO_attach(res,CircuitVisitor_ogv_visit);
	res->visitAnd = COO_attach(res,CircuitVisitor_ogv_visitAnd);
	res->visitXor = COO_attach(res,CircuitVisitor_ogv_visitXor);

	return res;
}

void OutputGateVisitor_Destroy(CircuitVisitor * cv) {
	CircuitVisitor c = 0;
	OGVImpl ogv = 0;
	OE oe = 0;

	if (!cv) return ;

	if (!*cv) return ;

	c = *cv;
	ogv = c->impl;
	oe = ogv->oe;

	COO_detach(c->visit);
	COO_detach(c->visitAnd);
	COO_detach(c->visitXor);

	HashMap_destroy(&ogv->gates_read);
	oe->putmem(c);
	oe->putmem(ogv);

	*cv = 0;
}
