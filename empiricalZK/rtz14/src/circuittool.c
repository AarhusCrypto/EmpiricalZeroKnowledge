/*
 * circuittool.c
 *
 *  Created on: Feb 1, 2015
 *      Author: rwl
 */

#include <osal.h>
#include <circuitparser.h>
#include <singlelinkedlist.h>

int main() {
	OE oe = OperatingEnvironment_LinuxNew();
	Tokenizer tk = FunCallTokenizer_New(oe);
	List circuit = 0;
	CircuitParser cp = CircuitParser_New(oe,tk);
	
	// TODO(rwl): Complete the circuit tool
	
	Tokenizer_Destrpy(&tk);
	CircuitParser_Destroy(&cp);
	OperatingEnvironment_LinuxDestroy(&oe);
	return 0;
}
