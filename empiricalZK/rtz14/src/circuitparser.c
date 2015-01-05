/*
 * circuitparser.c
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */



#include <circuitparser.h>
#include <coov3.h>



CircuitParser CircuitParser_New(OE oe, char * cstr) {
	CircuitParser parser = (CircuitParser)oe->getmem(sizeof(*parser));

	return parser;
}

typedef struct _fun_call_tokenizer_ {
	OE oe ;
	byte * data;
	uint ldata;
	uint cur;
	_Bool error;
	void (*parseAdd)(Token * tok);
	void (*parseMul)(Token * tok);
	void (*parseAnd)(Token * tok);
	void (*parseXor)(Token * tok);
	void (*parseNumber)(Token * tok);
	void (*parseString)(Token * tok);
} * FCT;

static inline int prefix(byte * a, byte * b, uint la, uint lb) {
	uint i = 0;

	if (la > lb) return 0;

	for(i = 0;i < la;++i) {
		if (a[i] != b[i]) return 0;
	}

	return 1;
}


uint atoui(byte * s, uint * res) {
 uint result = 0;
 uint ls = 0;

 while(s[ls] >= '0' && s[ls] <= '9') {
	 result *= 10;
	 result += (s[ls] - '0');
	 ++ls;
 }

 if (res) *res = result;

 return ls;

}

COO_DCL(Tokenizer,Token,nextToken);
COO_DEF_RET_NOARGS(Tokenizer,Token,nextToken) {
	FCT impl = (FCT)this->impl;
	char next = impl->data[impl->cur];
	Token res = {0};
	char white[] = {' ','\t','\n' };

	while (impl->cur < impl->ldata) {
		char c = impl->data[impl->cur];

		if (prefix((byte*)"XOR",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;
			res.type = XOR;
			return res;
		}

		if (prefix((byte*)"MUL",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;
			res.type = MUL;
			return res;
		}

		if (prefix((byte*)"AND",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;
			res.type = AND;
			return res;
		}

		if (prefix((byte*)"INV",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;
			res.type = XOR;
			return res;
		}

		if (c >= '0' && c <= '9') {
			res.type = NUM;
			impl->cur += atoui(&impl->data[impl->cur], &res.value.num);
			return res;
		}

		impl->cur++;
	}
 return res;
}

Tokenizer FunCallTokenizer_New(OE oe, byte * data, uint ldata) {
	FCT impl = 0;
	Tokenizer tk = (Tokenizer)oe->getmem(sizeof(*tk));
	if (!tk) return 0;

	impl = oe->getmem(sizeof(*impl));
	if (!impl) goto error;

	impl->oe = oe;
	impl->data = data;
	impl->ldata = ldata;
	impl->cur = 0;
	tk->impl = impl;
	COO_ATTACH(Tokenizer,tk,nextToken);
	return tk;
	error:
	FunCallTokenizer_Destroy(&tk);
	return 0;
}


void FunCallTokenizer_Destroy(Tokenizer * tk) {

}

Tokenizer AsmLikeTokenizer_New(OE oe, byte * data) {
	oe->syslog(OSAL_LOGLEVEL_WARN,"AsmLike tokenizer not implemented. Program may crash.");
	return 0;
}
void Tokenizer_Destroy(Tokenizer * tokenizer) {

}
