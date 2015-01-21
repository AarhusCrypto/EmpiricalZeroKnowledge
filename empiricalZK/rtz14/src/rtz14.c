#include <rtz14.h>
#include <coov3.h>
#include <singlelinkedlist.h>
#include <fs.h>
#include <circuitparser.h>
#include <circuit_analyser.h>
#include <map.h>
#include <emiter.h>
#include <carena.h>
#include <rnd.h>
#include <linear_proof.h>

typedef struct _rtz14_impl_ {
	OE oe;
	Data witness;
	Rnd rnd;
} * Rtz14Impl;

typedef struct _xor_commit_result {
	byte * m0;
	uint lm0;
	byte * m1;
	uint lm1;
	byte * commitment;
	uint lcommitment;
} * XORcommitResult;


static void commit(OE oe, Data out, Data message) {
 // TODO(rwz): implement this commit method using sha 512 assmebler version from the paper.
}

static XORcommitResult xor_commit(OE oe, Rnd rnd, byte * circuit_string, uint lcircuit_string) {
	XORcommitResult res = oe->getmem(sizeof(*res));
	return res;
}

static inline uint read_bit(byte * bit_str,uint idx) {
	uint byte_index = idx/8;
	uint bit_index_in_byte = idx % 8;
	return (bit_str[byte_index] & (0x01 << bit_index_in_byte)) != 0;
}

static byte * build_delta_string(OE oe, List proof_tasks) {
	// TODO(rwz): implement this.
 return 0;
}


COO_DCL(Rtz14,bool,executeProof, List circuit, byte * witness, char * ip, uint port);

COO_DEF_RET_ARGS(Rtz14, bool, executeProof,
		List circuit; byte * witness; char * ip; uint port;,
		circuit, witness, ip,port) {

	Rtz14Impl impl = (Rtz14Impl)this->impl;
	OE oe = impl->oe;
	Map input_gates = 0;
	CircuitVisitor emitter = 0;
	CircuitVisitor igv = InputGateVisitor_New(oe);
	EmiterResult emitter_res = 0;
	byte * emitted_circuit = 0;
	CArena conn = 0;
	CircuitVisitor proof_task_builder = 0;
	List proof_tasks = 0;
	if (!igv) return False;

	input_gates = igv->visit(circuit);
	if (!input_gates) return False;

	emitter = EvaluationStringEmitter_New(oe, input_gates, impl->witness->data);
	emitter_res = emitter->visit(circuit);

	conn = CArena_new(oe);

	if (ip == 0) {
		// -----------------------------
		// ----------- Prover ----------
		// -----------------------------
		MpcPeer verifier = 0;
		byte and_challenge[1] = {0};
		Data challenge_commitment = Data_new(oe,64);
		XORcommitResult xom = 0;
		Data epsilon = Data_new(oe,1);
		byte * delta = 0;
		Data judgement = Data_new(oe,8);

		if (witness == 0) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"No witness !");
			return False;
		}

		// inform the user what is happening
		oe->p("I am the Prover: Waiting for the Verifier to connect");
		conn->listen_wait(1,2020);
		oe->p("Verifier connected... ");
		verifier = conn->get_peer(0);

		// ---------------------------------------------
		// The Rtz14 protocol starts here for the prover
		// ---------------------------------------------

		// receive commitment to challenge from verifier.
		verifier->receive(challenge_commitment);

		// build xor commit to the emitted string
		xom = xor_commit(oe,impl->rnd,emitter_res->emitted_string, emitter_res->lemitted_string);

		// send the emitted string
		verifier->send(Data_shallow(emitter_res->emitted_string,emitter_res->lemitted_string));

		// receive and and_challenge b
		verifier->receive(Data_shallow(and_challenge,1));

		// send permutation or majority tests
		if (and_challenge[0] == 0) {
			//verifier->send(Data_shallow(emitter_res->perms,emitter_res->lperms));
			//proof_task_builder = ProofTaskBuilder_New(oe,and_challenge[0],emitter_res->perms, emitter_res->lperms);
		} else {
			verifier->send(Data_shallow(emitter_res->major,emitter_res->lmajor));
		}

		// build linear proof tasks

		proof_tasks = proof_task_builder->visit(circuit);

		// build delta string
		delta = build_delta_string(oe,proof_tasks);

		// send delta string to verifier
		verifier->send(Data_shallow(delta,proof_tasks->size()));

		// receive epsilon
		verifier->receive(epsilon);

		// send the m_epsilon string to the verifier
		if (epsilon->data[0] == 0) {
			verifier->send(Data_shallow(xom->m0,xom->lm0));
		} else {
			verifier->send(Data_shallow(xom->m1,xom->lm1));
		}

		// receive the string accept/reject from the verifier
		verifier->receive(judgement);

		// tell result to prover.
		oe->p("The verifier says: ");
		oe->p((char *)judgement->data);

		//TODO(rwz): clean up

	} else {
		// -----------------------------
		// ---------- Verifier ---------
		// -----------------------------

		// ---- The Rtz protocol starts here for the verifier ---

		bool accept = False;
		MpcPeer prover = 0;
		Data challenge_commitment = Data_new(oe,64);
		Data challenge = Data_new(oe,1);
		Data commitment_to_circuit = 0;
		Data and_challenge = Data_new(oe,1);
		Data permajor = 0;
		Data delta = 0;
		Data m_challenge = 0;
		uint i = 0, no_ands = 0;

		impl->rnd->rand(challenge->data,challenge->ldata);
		challenge->data[0] = challenge->data[0] % 2;
		commit(oe, challenge_commitment,challenge);


		impl->rnd->rand(and_challenge->data,and_challenge->ldata);
		and_challenge->data[0] = (and_challenge->data[0] % 2);

		conn->connect(ip,port);
		prover = conn->get_peer(0);

		if (!prover) {
			oe->p("No prover sorry, leaving.");
			return -2;
		}

		// send challenge commitment
		prover->send(challenge_commitment);

		// receive the xor-commitment to the circuit
		prover->receive(commitment_to_circuit);

		// send the and challenge
		prover->send(and_challenge);

		// count number of ands to anticipate size permajor
		for(i = 0; i < circuit->size();++i) {
			Gate g = circuit->get_element(i);
			if (g->type == G_AND)
				no_ands ++;
		}

		// allocate permajor
		permajor = Data_new(oe,3*no_ands);

		// receive permutation or majority tests
		prover->receive(permajor);

		// compute proof tasks the prover must do
		proof_task_builder = ProofTaskBuilder_New(oe,and_challenge->data[0],0,0);
		proof_tasks = proof_task_builder->visit(circuit);


		// --- Receive Delta ---
		delta = Data_new(oe,proof_tasks->size());
		prover->receive(delta);

		// send to committed to challenge from the beginning
		prover->send(challenge);

		// receive m_{challange}
		m_challenge = Data_new(oe,(circuit->size()*3+no_ands*3+7)/8);
		prover->receive(m_challenge);

		accept = True;
		for(i = 0; i < proof_tasks->size();++i) {
			byte xor = 0;
			uint j = 0;
			ProofTask cur = 0;//proof_tasks->get_element(i);
			List incidies = cur->indicies;
			for(j = 0;j < incidies->size();++j) {
				uint index_j = (uint)incidies->get_element(j);
				xor ^= read_bit(m_challenge->data,index_j);
			}

			if (xor != (byte)read_bit(delta->data,i)) {
				accept = False;
			}
		}

		if (accept) {
			prover->send(Data_shallow((byte*)"accept ",8));
			oe->p("Verifier accepted proof.");
		} else {
			prover->send(Data_shallow((byte*)"reject ",8));
			oe->p("Verifier rejected proof");
		}

		oe->p("Verifier done");

	}

	// close all connections.
	CArena_destroy(&conn);

	return True;
	error:
	return False;
}


Rtz14 Rtz14_New(OE oe, Rnd rnd) {

	// Instance
	Rtz14 res = (Rtz14)oe->getmem(sizeof(*res));
	Rtz14Impl impl = 0;

	if (!res) return 0;

	impl = (Rtz14Impl)oe->getmem(sizeof(*impl));
	if (!impl) return 0;

	res->impl = impl;

	// populate impl
	impl->oe = oe;
	impl->rnd = rnd;

	// setup interface functions
	COO_ATTACH(Rtz14,res,executeProof);

	return res;
}

void Rtz14_Destroy(Rtz14 * instance) {
	Rtz14 i = 0;
	OE oe = 0;
	Rtz14Impl impl = 0;

	if (!instance) return;
	i = *instance;
	impl = i->impl;

	oe = impl->oe;

	COO_DETACH(i,executeProof);

	oe->putmem(i);
	oe->putmem(impl);

	*instance = 0;
}
