#include <rtz14.h>
#include <coov3.h>


typedef struct _rtz14_impl_ {

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
	Rtz14 res = (Rtz14)oe->getmem(sizeof(*res));
	COO_ATTACH(Rtz14,res,isProver);
	COO_ATTACH(Rtz14,res,executeProof);
	return res;
}

void Rtz14_Destroy(Rtz14 * instance) {
 Rtz14 i = 0;
 if (!instance) return;
 i = *instance;

}
