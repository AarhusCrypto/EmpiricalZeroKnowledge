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

COO_DCL(CommitmentScheme, Data, dummy_open, Data commitment);
COO_DEF_RET_ARGS(CommitmentScheme, Data, dummy_open, Data commitment;,commitment) {
	OE oe = this->impl;
	return Data_copy(oe,commitment);
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

CommitmentScheme Sha512BasedUcScheme_New(OE oe, Rnd rnd) {
	oe->syslog(OSAL_LOGLEVEL_FATAL, "Sha512BasedUcScheme is not implemented yet.");
	return  0;
}

void Sha512BasedUcScheme_Destroy(CommitmentScheme * scheme) {

}
