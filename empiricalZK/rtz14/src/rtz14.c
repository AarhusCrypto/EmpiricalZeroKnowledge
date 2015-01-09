#include <rtz14.h>
#include <coov3.h>
#include <singlelinkedlist.h>
#include <fs.h>
#include <circuitparser.h>
#include <circuit_analyser.h>
#include <map.h>
#include <emitter.h>
#include <carena.h>

typedef struct _rtz14_impl_ {
	OE oe;
	Data witness;
} * Rtz14Impl;

COO_DCL(Rtz14,bool,executeProof, List circuit, byte * witness, char * ip, uint port);
COO_DEF_RET_ARGS(Rtz14, bool, executeProof,
		List circuit; byte * witness; char * ip; uint port;,
		circuit, witness, ip,port) {
	Rtz14Impl impl = (Rtz14Impl)this->impl;
	OE oe = impl->oe;
	Map input_gates = 0;
	CircuitVisitor emitter = 0;
	CircuitVisitor aux_and_data_builder = 0;
	CircuitVisitor igv = InputGateVisitor_New(oe);
	byte * emitted_circuit = 0;
	CArena conn = 0;
	if (!igv) return False;

	input_gates = igv->visit(circuit);
	if (!input_gates) return False;

	emitter = EvaluationStringEmitter_New(oe, input_gates, impl->witness->data);
	emitted_circuit = emitter->visit(circuit);



	conn = CArena_new(oe);
	if (ip == 0) {
		MpcPeer verifier = 0;
		byte challenge[1] = {0};
		if (witness == 0) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"No witness !");
			return False;
		}
		oe->p("I am the Prover: Waiting for the Verifier to connect");
		conn->listen_wait(1,2020);
		oe->p("Verifier connected... ");
		verifier = conn->get_peer(0);

		// The Rtz14 protocol starts here

		// commit to the emitted_circuit string with additional info.

		// Sigma protocol pattern

		verifier->send(Data_shallow(emitted_circuit,3*circuit->size()));

		verifier->receive(Data_shallow(challenge,1));

		verifier->send(Data_shallow(emitted_circuit,3*circuit->size()));

	} else {

	}

	return True;
	error:
	return False;
}


static Data build_circuit_string(OE oe, char * circuit_file) {
	uint filesize = read_file_size(circuit_file);
	byte * buffer = oe->getmem(filesize);
	uint lbuffer = 0;
	CircuitParser cp = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	uint count_and_gates = 0, count_xor_gates = 0;
	Data result = 0;
	CircuitVisitor poc = 0, igv = 0;
	Map input_gates = 0;
	List input_gate_addresses = 0;
	int i = 0;


	if (!buffer) return 0;
	lbuffer = read_entire_file(circuit_file, buffer, filesize);

	tk = FunCallTokenizer_New(oe);
	if (!tk) goto error;

	cp = CircuitParser_New(oe, tk);
	if (!cp) goto error;

	circuit = cp->parseSource(buffer,lbuffer);
	if (!circuit) goto error;

	poc->visit(circuit);
	input_gates = igv->visit(circuit);
	input_gate_addresses = input_gates->get_keys();


	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);

	for(i = 0; i < circuit->size();++i) {
		Gate g = circuit->get_element(i);
		if (g->type == G_AND) count_and_gates += 1;
		if (g->type == G_XOR) count_xor_gates += 1;
	}

	result = Data_new(oe, 3*count_xor_gates + 6*count_and_gates);

	return result;
	error:
	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);
	oe->putmem(buffer);
	Data_destroy(oe,&result);
	return 0;
}

Rtz14 Rtz14_New(OE oe) {

	// Instance
	Rtz14 res = (Rtz14)oe->getmem(sizeof(*res));
	Rtz14Impl impl = 0;

	if (!res) return 0;

	impl = (Rtz14Impl)oe->getmem(sizeof(*impl));
	if (!impl) return 0;

	res->impl = impl;

	// populate impl
	impl->oe = oe;

	// setup interface functions
	COO_ATTACH(Rtz14,res,executeProof);

	return res;
}

void Rtz14_Destroy(Rtz14 * instance) {
	Rtz14 i = 0;
	if (!instance) return;
	i = *instance;

}
