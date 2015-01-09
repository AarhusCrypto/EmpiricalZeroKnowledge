/*
 * prover.c
 *
 *  Created on: Jan 8, 2015
 *      Author: rwl
 */


#include <osal.h>
#include <rtz14.h>
#include <fs.h>
#include <utils/options.h>
#include <encoding/hex.h>
#include <stdlib.h>
#include <circuitparser.h>
#include <emitter.h>


static uint load_buffer(OE oe, byte ** buffer, char * filename) {
	uint lbuffer = 0;
	lbuffer = read_file_size(filename);
	*buffer = oe->getmem(lbuffer);
	if (!*buffer) {
		oe->syslog(OSAL_LOGLEVEL_FATAL,"Out of memory.");
		return -1;
	}
	lbuffer = read_entire_file(filename, *buffer, lbuffer);
	return lbuffer;
}

static byte * decode_witness(OE oe, Map options) {

	byte * res = 0;
	byte * wit_cstr= options->get("witness");
	uint lwit_cstr = 0;
	if (!wit_cstr) return 0;
	while(wit_cstr[++lwit_cstr]);
	if (lwit_cstr % 2 != 0) {
		oe->syslog(OSAL_LOGLEVEL_FATAL,"Witness is present but must be a valid hex string (even length).");
		exit(-1);
	}

	res = oe->getmem(lwit_cstr/2+1);
	if (!res) return 0;

	hs2bs((char*)wit_cstr,res,lwit_cstr/2);
	return res;
}

static void clean_up_circuit(OE oe, List circuit) {
	uint i = 0;
	if (!circuit) return;

 	for(i = 0; i < circuit->size();++i) {
 		Gate g = circuit->get_element(i);
 		zeromem(g, sizeof(*g));
 		oe->putmem(g);
	}

 	SingleLinkedList_destroy(&circuit);
}

static uint decode_port(OE oe, Map options) {
	char * port_s = 0;
	uint port = 0;
	if (!options->contains("port")) {
		oe->p("No port argument given resorting to the default port of 2020");
		return 2020;
	}

	port_s = (char*)options->get("port");

	atoui(port_s,&port);

	return port;
}

int main(int argc, char ** argv) {

	OE oe = OperatingEnvironment_LinuxNew();
	byte * buffer = 0;
	uint lbuffer = 0;
	byte * witness = 0;
	uint port = 0;
	Map options = 0;
	Rtz14 zk = 0;

	if (!oe) {
		printf("Unsupported platform :( unable to instantiate Operating System Abstract.\n");
		return -1;
	}

	options = Options_New(oe,argc,argv);
	if (!options) {
		oe->syslog(OSAL_LOGLEVEL_FATAL,"unable to parse command line arguments :(");
		return -2;
	}

	zk =  Rtz14_New(oe);
	if (!zk) {
		oe->syslog(OSAL_LOGLEVEL_FATAL,"unable to create Rtz14 instance.");
		return -3;
	}

	if (options->contains("circuit") && options->contains("port")) {
		bool success = 0;
		Tokenizer tk = 0;
		CircuitParser cp = 0;
		List circuit = 0;


		// create tokenizer that accept the langauage of
		// JBN AES circuit from Nigel et al.
		tk = FunCallTokenizer_New(oe);

		// create a parser
		cp = CircuitParser_New(oe,tk);
		if (!cp) return -4;

		// load entire circuit into memory
		lbuffer = load_buffer(oe,&buffer,options->get("circuit"));
		if (!buffer) return 0;

		// decode witness
		witness = decode_witness(oe,options);

		// decode port
		port = decode_port(oe,options);

		// parse buffer to a circuit (list of gates)
		circuit = cp->parseSource(buffer,lbuffer);

		// buffer no longer needed free up space
		(oe->putmem(buffer),buffer=0,lbuffer=0);

		// act as prover or verifier depending on arguments given.
		success = zk->executeProof(circuit, witness,
				                   (char*)options->get("ip"), port);

		// report to the user the result
		if (success == True) {
			oe->p("Verifier accepts the proof.");
		} else {
			oe->p("Proof was rejected.");
		}

		// clean up
		CircuitParser_Destroy(&cp);
		clean_up_circuit(oe,circuit);
		Rtz14_Destroy(&zk);
	}

	// and we are done
	OperatingEnvironment_LinuxDestroy(&oe);
	return 0;
}
