/*
 * circuitparser.c
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */



#include <circuitparser.h>
#include <coov3.h>
#include <osal.h>
#include <singlelinkedlist.h>

typedef struct _circuit_parser_impl_ {
	Gate (*parseAnd)();
	Gate (*parseXor)();
	Gate (*parseInv)();
	OE oe;
	Tokenizer tokenizer;
} * CPI;

void Circuit_Destroy(OE oe, List * circuit) {
	List c = 0;
	Gate g = 0;
	uint i = 0;
	if (!circuit) return;
	c = *circuit;
	if (!c) return;

	for(i = 0;i < c->size();++i) {
		g = c->get_element(i);
		oe->putmem(g);
	}
	SingleLinkedList_destroy(circuit);
}

COO_DCL(CPI,Gate,parseAnd);
COO_DEF_RET_NOARGS(CPI, Gate, parseAnd) {
	char b[64] = {0};
	Token dst = {0}, op1 = {0}, op2 = {0};
	Gate g = 0;
	this->tokenizer->nextToken(&dst);
	this->tokenizer->nextToken(&op1);
	this->tokenizer->nextToken(&op2);
	// AND(dst,op1,op2,index) where we do
	// not care about index therefore it
	// is read and ignored.
	this->tokenizer->nextToken(0);

	if (dst.type != NUM) {
		osal_sprintf(b,"Error: unexpected token at line %u character %u.",dst.location.line,dst.location.linepos);
		return 0;
	}

	if (op1.type != NUM) {
		osal_sprintf(b,"Error: unexpected token at line %u character %u.",dst.location.line,dst.location.linepos);
		return 0;
	}

	if (op2.type != NUM) {
		osal_sprintf(b,"Error: unexpected token at line %u character %u.",dst.location.line,dst.location.linepos);
		return 0;
	}

	g = this->oe->getmem(sizeof(*g));
	g->type = G_AND;
	g->dst = dst.value.num;
	g->op1 = op1.value.num;
	g->op2 = op2.value.num;
	return g;
}

COO_DCL(CPI, Gate , parseXor);
COO_DEF_RET_NOARGS(CPI, Gate , parseXor) {
	Token dst = {0}, op1 = {0}, op2 = {0};
	Gate g = 0;
	this->tokenizer->nextToken(&dst);
	this->tokenizer->nextToken(&op1);
	this->tokenizer->nextToken(&op2);
	// XOR(dst,op1,op2,index) where we do
	// not care about index therefore it
	// is read and ignored.
	this->tokenizer->nextToken(0);

	if (dst.type != NUM) {
		char b[64] = {0};
		osal_sprintf(b,"Error: unexpected token at line %u character %u.",dst.location.line,dst.location.linepos);
		this->oe->p(b);
		return 0;
	}

	if (op1.type != NUM) {
		char b[64] = {0};
		osal_sprintf(b,"Error: unexpected token at line %u character %u.",dst.location.line,dst.location.linepos);
		this->oe->p(b);
		return 0;
	}

	if (op2.type != NUM) {
		char b[64] = {0};
		osal_sprintf(b,"Error: unexpected token at line %u character %u.",dst.location.line,dst.location.linepos);
		this->oe->p(b);
		return 0;
	}

	g = this->oe->getmem(sizeof(*g));
	g->type = G_XOR;
	g->dst = dst.value.num;
	g->op1 = op1.value.num;
	g->op2 = op2.value.num;
	return g;
}

// Our AST basis for analysis is XOR and AND. We do not accept INV however,
// INV(x) is equivalent to XOR(x,1) so we build XOR(x,1) instead.
COO_DCL(CPI, Gate, parseInv);
COO_DEF_RET_NOARGS(CPI,Gate,parseInv) {
	OE oe = this->oe;
	Gate g = (Gate)oe->getmem(sizeof(*g));
	Token dst = {0}, src = {0};

	this->tokenizer->nextToken(&dst);
	this->tokenizer->nextToken(&src);

	if (dst.type != NUM) {
		char b[64] = {0};
		osal_sprintf(b, "Error: expected a number at line %u character %u.",dst.location.line,dst.location.linepos);
		oe->p(b);
		return 0;
	}

	if (src.type != NUM) {
		char b[64] = {0};
		osal_sprintf(b, "Error: expected a number at line %u character %u.",src.location.line,src.location.linepos);
		oe->p(b);
		return 0;

	}

	g->type = G_XOR;
	g->dst = dst.value.num;
	g->op1 = -1;
	g->op2 = src.value.num; // TODO(rwz): We add here the special Heap address -1 which is UINT_MAX
	// this makes the use of that heap address unavailable to circuit definitions.
	return g;
}

static void print_token(OE oe, Token tok) {
	char b[128] = {0};
	switch(tok.type) {
	case AND:
		osal_sprintf(b,"Token[AND { line: %u, linepos: %u, offset: %u }]",
				tok.location.line,
				tok.location.linepos,
				tok.location.offset);
		break;
	case XOR:
		osal_sprintf(b,"Token[XOR { line: %u, linepos: %u, offset: %u }]",
				tok.location.line,
				tok.location.linepos,
				tok.location.offset);
		break;
	case NUM:
		osal_sprintf(b,"Token[NUM { line: %u, linepos: %u, offset: %u }]",
				tok.location.line,
				tok.location.linepos,
				tok.location.offset);
		break;
	case DONE:
		osal_sprintf(b,"Token[DONE { line: %u, linepos: %u, offset: %u }]",
				tok.location.line,
				tok.location.linepos,
				tok.location.offset);
		break;
	case ERROR:
		osal_sprintf(b,"Token[ERROR { line: %u, linepos: %u, offset: %u }]",
				tok.location.line,
				tok.location.linepos,
				tok.location.offset);
		break;
	default:
		osal_sprintf(b,"Token[ UNKNOWN ! { line: %u, linepos: %u, offset: %u }]",
				tok.location.line,
				tok.location.linepos,
				tok.location.offset);
		break;

	}
	oe->p(b);
}

#define Error(OE,MSG,...) {                  \
	byte ___b___[512] = {0};                 \
    osal_sprintf(___b___,(MSG),__VA_ARGS__); \
    oe->syslog(OSAL_LOGLEVEL_FATAL,___b___); \
  }

COO_DCL(CircuitParser, List, parseSource, byte * src, uint lsrc);
COO_DEF_RET_ARGS(CircuitParser, List, parseSource, byte * src; uint lsrc;, src,lsrc) {
	char msg[64] = {0};
	CPI cpi = (CPI)this->impl;
	OE oe = cpi->oe;
	List result = SingleLinkedList_new(cpi->oe);
	Token tok = {0};

	if (lsrc == 0) return result;

	cpi->tokenizer->start(src,lsrc);

	do {
		RC rc = cpi->tokenizer->nextToken(&tok);
		switch(tok.type) {
		case XOR: {
			Gate g = cpi->parseXor();
			if (g == 0) {
				Error(oe,"Error: [%u:%u] could not parse XOR gate.",tok.location.line,tok.location.linepos);
				return result;
			}
			result->add_element(g);
		}
		break;
		case AND: {
			Gate g = cpi->parseAnd();
			if (g == 0) {
				Error(oe,"Error: [%u:%u] could not parse AND gate.",tok.location.line,tok.location.linepos);
				return result;
			}
			result->add_element(g);
		}
		break;
		case INV: {
			Gate g = cpi->parseInv();
			if (g == 0) {
				Error(oe,"Error: [%u:%u] could not parse INV gate.",tok.location.line,tok.location.linepos);
				return result;
			}
			result->add_element(g);
			break;
		}
		case DONE:
			break;
		case ERROR:
			Error(oe,"Error: [%u:%u] Syntax error",tok.location.line,tok.location.linepos);
			break;
		default: {
			/*
			print_token(oe,tok);
			osal_sprintf(msg, "Skipping token: at line %u character %u - unexpected token.",
					tok.location.line+1, tok.location.linepos+1);
			oe->syslog(OSAL_LOGLEVEL_WARN,msg);
			*/
			break;
		}
		}

	} while(tok.type != DONE && tok.type != ERROR);
	return result;
}

CircuitParser CircuitParser_New(OE oe, Tokenizer tk) {
	CircuitParser parser = 0;
	CPI impl = 0;

	parser = (CircuitParser)oe->getmem(sizeof(*parser));
	if (!parser) return 0;

	impl = (CPI)oe->getmem(sizeof(*impl));
	if (!impl) goto error;

	impl->tokenizer = tk;
	impl->oe = oe;

	parser->impl = impl;

	COO_ATTACH(CircuitParser, parser, parseSource);
	COO_ATTACH(CPI, impl, parseXor);
	COO_ATTACH(CPI, impl, parseAnd);
	COO_ATTACH(CPI, impl, parseInv);

	return parser;
	error:
	CircuitParser_Destroy(&parser);
	return 0;
}

void CircuitParser_Destroy(CircuitParser * parser) {
	CircuitParser p = 0;
	CPI cpi = 0;OE oe = 0;

	if (!parser) return;
	p = *parser;

	cpi = (CPI)p->impl;
	if (!cpi) return;

	oe = cpi->oe;
	COO_DETACH(p,parseSource);
	COO_DETACH(cpi,parseAnd);
	COO_DETACH(cpi,parseXor);
	COO_DETACH(cpi,parseInv);

	oe->putmem(p);
	oe->putmem(cpi);
	*parser = 0;
}

typedef struct _fun_call_tokenizer_ {
	OE oe ;
	byte * data;
	uint ldata;
	uint line;
	uint linepos;
	uint cur;
	_Bool error;
} * FCT;

static inline int prefix(byte * a, byte * b, uint la, uint lb) {
	uint i = 0;

	if (la > lb) return 0;

	for(i = 0;i < la;++i) {
		if (a[i] != b[i]) return 0;
	}

	return 1;
}

#define SET_LOCATION(tok,fct)\
		tok.location.line = fct->line; \
		tok.location.linepos = fct->linepos; \
		tok.location.offset = fct->cur;


//a name is [a-zA-Z$_][a-zA-Z$_0-9]*
static uint read_name(byte *d, uint ld, uint pos) {
	uint i = pos-1;
	while (++i < ld) {
		if (d[i] >= 'a' && d[i] <= 'z') continue;
		if (d[i] >= 'A' && d[i] <= 'Z') continue;
		if (d[i] == '$') continue;
		if (d[i] == '_') continue;
		if (i > pos) {
			if (d[i] >= '0' && d[i] <= '9') continue;
		}
		break;
	}
	return i-pos;
}

COO_DCL(Tokenizer,RC,nextToken,Token * tok);
COO_DEF_RET_ARGS(Tokenizer,RC,nextToken, Token * tok;, tok) {
	FCT impl = (FCT)this->impl;
	Token res = {0};
	OE oe = impl->oe;

	while (impl->cur < impl->ldata) {
		char c = impl->data[impl->cur];
		uint name_len = read_name(impl->data,impl->ldata,impl->cur);
		SET_LOCATION(res,impl)

		if (name_len > 3 ) {
			//printf("%s\nldata: %u cur: %u len:%u\n",impl->data,impl->ldata,impl->cur,name_len);
			impl->cur += name_len;
			impl->linepos += name_len;
			continue;
		}

		if (prefix((byte*)"XOR",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;impl->linepos += 3;
			res.type = XOR;
			if (tok) *tok = res;
			return RC_OK;
		}

		if (prefix((byte*)"MUL",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;impl->linepos += 3;
			res.type = MUL;
			if (tok) *tok = res;
			return RC_OK;
		}

		if (prefix((byte*)"AND",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;impl->linepos += 3;
			res.type = AND;
			if (tok) *tok = res;
			return RC_OK;
		}

		if (prefix((byte*)"INV",impl->data + impl->cur,3,impl->ldata)) {
			impl->cur += 3;impl->linepos += 3;
			res.type = INV;
			if (tok) *tok = res;
			return RC_OK;
		}

		if (c >= '0' && c <= '9') {
			uint advance = 0;
			res.type = NUM;
			advance = atoui(&impl->data[impl->cur], &res.value.num);
			impl->linepos += advance;
			impl->cur += advance;
			if (tok) *tok = res;
			return RC_OK;
		}

		if (c == '\n') {
			impl->linepos = 0;
			impl->line += 1;
		} else {
			impl->linepos++;
		}

		impl->cur++;
	}
	if (tok) *tok = res;
	return RC_OK;
}

COO_DCL(Tokenizer,void,start,byte * src, uint lsrc);
COO_DEF_NORET_ARGS(Tokenizer,start,byte *src; uint lsrc;, src,lsrc) {
	FCT impl = this->impl;
	impl->data = src;
	impl->ldata = lsrc;
	impl->cur = 0;
}

Tokenizer FunCallTokenizer_New(OE oe) {
	FCT impl = 0;
	Tokenizer tk = (Tokenizer)oe->getmem(sizeof(*tk));
	if (!tk) return 0;

	impl = oe->getmem(sizeof(*impl));
	if (!impl) goto error;

	impl->oe = oe;
	impl->cur = 0;
	tk->impl = impl;
	COO_ATTACH(Tokenizer,tk,nextToken);
	COO_ATTACH(Tokenizer,tk,start);
	return tk;
	error:
	FunCallTokenizer_Destroy(&tk);
	return 0;
}


void FunCallTokenizer_Destroy(Tokenizer * tk) {
	FCT fct = 0;
	Tokenizer t = 0;
	OE oe = 0;

	if (!tk) return;

	t = *tk;
	if (!t->impl) return;


	COO_DETACH(t, nextToken);
	COO_DETACH(t, start);

	fct = (FCT)t->impl;
	oe = fct->oe;

	oe->putmem(t);
	oe->putmem(fct);
}

Tokenizer AsmLikeTokenizer_New(OE oe) {
	oe->syslog(OSAL_LOGLEVEL_WARN,"AsmLike tokenizer not implemented. Program may crash.");
	return 0;
}
