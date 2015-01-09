/*
 * circuitparser.h
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */

#ifndef RTZ14_INCLUDE_CIRCUITPARSER_H_
#define RTZ14_INCLUDE_CIRCUITPARSER_H_
#include <osal.h>
#include <singlelinkedlist.h>
/**
 * The circuit parser can read a circuit in the specified circuit language.
 *
 * Created with a tokenizer that recognizes the tokens of the language the parser
 * will build an AST e.g. a list of gates.
 *
 */
typedef struct _circuit_parser_ {
 List (*parseSource)(byte * src, uint lsrc);
 void * impl;
} * CircuitParser;

typedef enum {
	G_AND, G_XOR, G_ONE, G_ZERO
} GType;

/**
 * Abstract Syntax, we work in the AND, XOR basis
 */
typedef struct _gate_ {
	GType type;
	uint dst;
	uint op1;
	uint op2;
} * Gate;

typedef struct _source_map_ {

} * SourceMap;

// Tokens that we support in any tokenizer.
typedef enum {
	/**
	 * The DONE token is return by the
	 * tokenizer when end of file or stream is reached.
	 */
	DONE,
	/**
	 * The ERROR token is return when a syntax error is encountered.
	 */
	ERROR,
	/**
	 * Circuit tokens below.
	 */
	ADD,MUL,NAND,OR,XOR,AND,INV,CSTR,NUM
} TokenType;

/**
 * Token interface we have a the type, the location in the source and
 * data connected to the token, e.g. if it is a number token the integer
 * value is there or a string token then the literal string is there.
 */
typedef struct _token_ {
	// type
	TokenType type;

	// loc
	struct location {
		uint offset;
		uint line;
		uint linepos;
	} location;

	// value
  union {
	  char * str;
	  uint num;
  } value;
} Token;

/**
 * A tokenizer splits a sequence of characters into
 * tokens. Interface designed to support streaming.
 */
typedef struct _tokenizer_ {
	/**
	 * Reset the tokenizer to the data in {src}.
	 */
	void (*start)(byte * src, uint lsrc);

	/**
	 * Advance the tokenizer to the and including the next Token.
	 */
	RC (*nextToken)(Token * token);

	// Implementation details
	void * impl;
} * Tokenizer;

/**
 * The function call tokenizer supports the syntax in "spacls" files
 * by Nigel Smart et al. which is basically C++ function calls. This
 * tokenizer ignores everything that it does not recognize. In fact it
 * takes any string and reads either XOR, AND, INV and NUM and ignore
 * anything in between that is not recognized.
 *
 * \param oe - operating system abstraction layer
 *
 * \return fresh Tokenizer instance.
 *
 */
Tokenizer FunCallTokenizer_New(OE oe);

// Cleanup a function call tokenizer
void FunCallTokenizer_Destroy(Tokenizer * tk);

// Assembler like circuit language a al. that of
// Marcel Keller in his work with AES.
Tokenizer AsmLikeTokenizer_New(OE oe);

//clearup the assembler like tokenizer.
void AsmLikeTokenizer_Destroy(Tokenizer * tokenizer);

/**
 *  Create a circuit parser building an Abstract Syntax from
 *  the tokens understood by the {tokenizer} argument.
 *
 *  \param oe - os abstractio layer
 *  \param tokenizer - the tokenizer/scanner reading the source
 *
 *  \return a CircuitParser.
 */
CircuitParser CircuitParser_New(OE oe, Tokenizer tokenizer);

// clean up parser.
void CircuitParser_Destroy(CircuitParser * parser);
#endif /* RTZ14_INCLUDE_CIRCUITPARSER_H_ */
