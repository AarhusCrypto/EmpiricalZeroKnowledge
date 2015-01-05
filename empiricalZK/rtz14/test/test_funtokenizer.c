/*
 * test_funtokenizer.c
 *
 *  Created on: Jan 5, 2015
 *      Author: rwl
 */

#include <osal.h>
#include <circuitparser.h>
#include "test.h"




static int create_fun_tokenizer(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe,(byte*)"what",0);
	FunCallTokenizer_Destroy(&tk);
	return (tk != 0);
}

static int tokenize_xor(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe,(byte*)"XOR",3);
	Token t = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return t.type == XOR;
}

static int tokenize_and(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe,(byte*)"AND",3);
	Token t = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return t.type == AND;
}


static int tokenize_inv(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe,(byte*)"INV",3);
	Token t = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return t.type == XOR;
}

static int tokenize_num(OE oe) {
	char b[64] = {0};
	Tokenizer tk = FunCallTokenizer_New(oe,(byte*)"145",3);
	Token t = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return t.type == NUM && t.value.num == 145;

}

static int tokenize_xor_spec(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe, (byte*)"XOR(1,2,3);",11);
	Token xor = tk->nextToken();
	Token one = tk->nextToken();
	Token two = tk->nextToken();
	Token three = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return xor.type == XOR && one.type == NUM && one.value.num == 1 && two.type == NUM && two.value.num == 2 && three.type == NUM && three.value.num == 3;
}

static int tokenize_and_spec(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe, (byte*)"AND(1,2,3);",11);
	Token xor = tk->nextToken();
	Token one = tk->nextToken();
	Token two = tk->nextToken();
	Token three = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return xor.type == AND && one.type == NUM && one.value.num == 1 && two.type == NUM && two.value.num == 2 && three.type == NUM && three.value.num == 3;
}


static int tokenize_inv_spec(OE oe) {
	Tokenizer tk = FunCallTokenizer_New(oe, (byte*)"INV(1,2,3);",11);
	Token xor = tk->nextToken();
	Token one = tk->nextToken();
	Token two = tk->nextToken();
	Token three = tk->nextToken();
	FunCallTokenizer_Destroy(&tk);
	return xor.type == XOR && one.type == NUM && one.value.num == 1 && two.type == NUM && two.value.num == 2 && three.type == NUM && three.value.num == 3;
}

static int tokenize_file(OE oe) {
	uint lbuffer = 1060365;
	uint ands = 0, xors = 0, nums = 0, tokens = 0;
	Tokenizer tk = 0;
	Token tok = {0};
	byte * buffer = oe->getmem(lbuffer);
	uint fp = oe->open("file ../test/AES");
	oe->read(fp,buffer,&lbuffer);
	oe->close(fp);
	tk = FunCallTokenizer_New(oe, buffer,lbuffer);

	do {
		tok = tk->nextToken();
		if (tok.type == AND) ands +=1;
		if (tok.type == XOR) xors += 1;
		if (tok.type == NUM) nums += 1;
		tokens++;
	} while(tok.type != DONE);

	//DBG_P(oe,"\nANDS: %u\nXORS: %u\nNUMS: %u\nTOKENS: %u\n",ands,xors,nums,tokens);
	oe->putmem(buffer);
	return ands == 6881 && xors == 27552 && nums == 139136 && tokens == 173570;
}

Test tests[] = {
		{"Create fun tokenizer",create_fun_tokenizer},
		{"Tokenize XOR",tokenize_xor},
		{"Tokenize AND",tokenize_and},
		{"Tokenize INV",tokenize_inv},
		{"Tokenize NUMBER", tokenize_num},
		{"Tokenize XOR Spec",tokenize_xor_spec},
		{"Tokenize AND Spec",tokenize_and_spec},
		{"Tokenize INV Spec",tokenize_inv_spec},
		{"Tokenize Large File",tokenize_file}
};

TEST_MAIN
