/*
 * commitment.c
 *
 *  Created on: Jan 22, 2015
 *      Author: rwl
 */


#include <commitment.h>
#include <coov3.h>

COO_DCL(CommitmentScheme, Data, dummy_commit, Data message);
COO_DEF_RET_ARGS(CommitmentScheme, Data, dummy_commit, Data message;, message) {
	OE oe = this->impl;
	return Data_copy(oe,message);
}

COO_DCL(CommitmentScheme, Data, dummy_open, Data commitment,Data msg);
COO_DEF_RET_ARGS(CommitmentScheme, Data, dummy_open, Data commitment; Data msg;,commitment,msg) {
	return True;
}


CommitmentScheme DummyScheme_New(OE oe) {
	CommitmentScheme cs = (CommitmentScheme)oe->getmem(sizeof(*cs));
	if (!cs) return 0;

	COO_ATTACH_FN(CommitmentScheme,cs,commit,dummy_commit);
	COO_ATTACH_FN(CommitmentScheme,cs,open,dummy_open);

	cs->impl = oe;

	return cs;
}

void DymmyScheme_Destroy(CommitmentScheme * scheme) {
	CommitmentScheme s = 0;
	OE oe = 0;
	if (!scheme) return;

	s = *scheme;
	oe = s->impl;
	oe->putmem(s);
	*scheme = 0;
}

// TODO(rwl): When permission is given include this commitment scheme based on 
// sha512 by Nayuki.

typedef struct _sha512_uc_cs_impl_ {
	OE oe ;
	Rnd rnd;
} * Sha512CsImpl;




static inline
void ull2bs(ull u, byte * d) {
  uint i = 0;
  d[0] = ((ull)(u & ((ull)0xFF << 8*7))) >> 7*8;
  d[1] = ((ull)(u & ((ull)0xFF << 8*6))) >> 6*8;
  d[2] = ((ull)(u & ((ull)0xFF << 8*5))) >> 5*8;
  d[3] = ((ull)(u & ((ull)0xFF << 8*4))) >> 4*8;
  d[4] = (u & (0xFF << 8*3)) >> 3*8;
  d[5] = (u & (0xFF << 8*2)) >> 2*8;
  d[6] = (u & (0xFF << 8*1)) >> 1*8;
  d[7] = (u & (0xFF ));

}

static inline
void ulls2bs(ull * uls, uint luls, byte * result) {
  uint i = 0;

  if (!uls || !result) return ;

  for(i = 0; i < luls;++i) {
    ull2bs(uls[i],result+8*i);
  }
}

COO_DCL(CommitmentScheme,Data,sha512_commit,Data msg);
COO_DEF_RET_ARGS(CommitmentScheme, Data, sha512_commit, Data msg;, msg) {
	Sha512CsImpl impl = this->impl;
	Rnd rnd = impl->rnd;
	Data result = Data_new(impl->oe,80);
	ull hash[8] = {0};
	Data message_and_random = Data_new(impl->oe, msg->ldata+16);
	mcpy(message_and_random->data,msg->data,msg->ldata);
	rnd->rand(message_and_random->data+msg->ldata,16);
	//sha512_hash(message_and_random->data, message_and_random->ldata, hash);
	ulls2bs(hash, sizeof(hash)/sizeof(ull), result->data);


	mcpy(result->data+64,message_and_random->data+msg->ldata,16);
	Data_destroy(impl->oe, &message_and_random);
	return result;
}

COO_DCL(CommitmentScheme,bool, sha512_open, Data cmt,Data msg);
COO_DEF_RET_ARGS(CommitmentScheme,bool, sha512_open, Data cmt; Data msg;,cmt,msg) {
	Sha512CsImpl impl = this->impl;
	uint i = 0;
	Data message_and_random = 0;
	ull hash[8] = {0};
	Data real = 0;

	if (!msg) goto fail;

	if (!cmt) goto fail;

	message_and_random = Data_new(impl->oe,msg->ldata+16);
	mcpy(message_and_random->data,msg->data,msg->ldata);
	mcpy(message_and_random->data+msg->ldata,cmt->data+64,16);

	//sha512_hash(message_and_random->data,message_and_random->ldata,hash);
	real = Data_new(impl->oe,64);
	ulls2bs(hash,8,real->data);

	for(i = 0; i < real->ldata;++i) {
		if (cmt->data[i] != real->data[i]) goto fail;
	}
	Data_destroy(impl->oe, &real);
	Data_destroy(impl->oe, &message_and_random);
	return True;
	fail:
	Data_destroy(impl->oe, &real);
	return False;
}

CommitmentScheme Sha512BasedUcScheme_New(OE oe, Rnd rnd) {
	CommitmentScheme cs = (CommitmentScheme)oe->getmem(sizeof(*cs));
	Sha512CsImpl impl = 0;

	if (!cs) return 0;
	impl = (Sha512CsImpl)oe->getmem(sizeof(*impl));

	if (!impl) goto failure;
	impl->oe = oe;
	impl->rnd = rnd;
	cs->impl = impl;

	COO_ATTACH_FN(CommitmentScheme, cs, commit, sha512_commit);
	COO_ATTACH_FN(CommitmentScheme, cs, open, sha512_open);

	return cs;
	failure:
	if (impl) {
		Sha512BasedUcScheme_Destroy(&cs);
	} else {
		oe->putmem(cs);
	}
	return  0;
}

void Sha512BasedUcScheme_Destroy(CommitmentScheme * scheme) {
	CommitmentScheme c = 0;
	Sha512CsImpl i = 0;
	if (!scheme) return;

	if (!*scheme) return ;

	c = *scheme;
	i = c->impl;

	COO_DETACH(c,commit);
	COO_DETACH(c,open);
	i->oe->putmem(c);
	i->oe->putmem(i);
	*scheme = 0;
}
