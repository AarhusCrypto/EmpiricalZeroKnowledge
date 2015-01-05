/*
 * circuitparser.h
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */

#ifndef RTZ14_INCLUDE_CIRCUITPARSER_H_
#define RTZ14_INCLUDE_CIRCUITPARSER_H_
#include <osal.h>
/**
 * The circuit parser can read a circuit in the specified circuit language.
 *
 *
 *
 *
 */

typedef struct _circuit_parser_ {

 void * impl;
} * CircuitParser;

typedef enum {
	DONE,ADD,MUL,NAND,OR,XOR,AND,CSTR,NUM
} TokenType;

typedef struct _token_ {
	TokenType type;
  union {
	  char * str;
	  uint num;
  } value;
} Token;

typedef struct _tokenizer_ {
	Token (*nextToken)();
	void * impl;
} * Tokenizer;

Tokenizer FunCallTokenizer_New(OE oe, byte * data,uint ldata);
void FunCallTokenizer_Destroy(Tokenizer * tk);

Tokenizer AsmLikeTokenizer_New(OE oe, byte * data);
void Tokenizer_Destroy(Tokenizer * tokenizer);

CircuitParser CircuitParser_New(OE oe, char * cstr);
void CircuitParser_Destroy(CircuitParser * parser);
#endif /* RTZ14_INCLUDE_CIRCUITPARSER_H_ */
