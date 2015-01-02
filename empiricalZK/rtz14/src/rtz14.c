#include <rtz14.h>
#include <coov3.h>
#include <singlelinkedlist.h>

typedef struct _rtz14_impl_ {
 OE oe;
 Data * witness;
 List loaded_gates;
} * Rtz14Impl;

COO_DCL(Rtz14, bool, isProver);
COO_DEF_RET_NOARGS(Rtz14,bool,isProver) {
 return False;
}

COO_DCL(Rtz14,bool,executeProof,char * ip, uint port);
COO_DEF_RET_ARGS(Rtz14, bool, executeProof, char * ip; uint port;,ip,port) {
 return False;
}

Rtz14 Rtz14_New(OE oe, char * circuit_file, byte * witness) {

	// Instance
	Rtz14 res = (Rtz14)oe->getmem(sizeof(*res));
	Rtz14Impl impl = 0;

	if (!res) return 0;

	impl = (Rtz14Impl)oe->getmem(sizeof(*impl));
	if (!impl) return 0;

	res->impl = impl;

	// populate impl
	impl->oe = oe;
	impl->loaded_gates = SingleLinkedList_new(oe);
	impl->witness = 0;

	// setup interface functions
	COO_ATTACH(Rtz14,res,isProver);
	COO_ATTACH(Rtz14,res,executeProof);

	return res;
}

void Rtz14_Destroy(Rtz14 * instance) {
 Rtz14 i = 0;
 if (!instance) return;
 i = *instance;

}
