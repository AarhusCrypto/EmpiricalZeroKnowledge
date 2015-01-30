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
#include <commitment.h>
#include <datetime.h>

static inline byte get_bit(byte * bita, uint idx) {
	uint byte_idx = idx/8;
	uint bit_idx = idx-byte_idx*8;
	byte mask =  0x01 << bit_idx;
	return (bita[byte_idx] & mask) != 0;
}


// Debug helper to print a bit-string
#define AddCh(B,C,IDX) { (B)[(IDX)] = (C); (IDX) += 1; }
static void print_bit_string(OE oe, Data bs) {

	char buf[512] = {0};
	uint idx = 0;
	uint i = 0;
	uint length = (sizeof(buf)-2) > bs->ldata*8 ? bs->ldata*8 : sizeof(buf)-2;

	AddCh(buf,'\n',idx);
	for(i = 0; i < bs->ldata*8 && idx < length-4;++i) {
		byte bit = get_bit(bs->data,i);
		// add white space
		if (i > 0 && i % 24 == 0) {
			AddCh(buf,'\n',idx);
		} else {
			if (i > 0 && i % 8 == 0) {
				AddCh(buf,' ',idx);
			}
		}
		if (bit == 1) {
			AddCh(buf,'1',idx);
		} else {
			AddCh(buf,'0',idx);
		}
	}
	AddCh(buf,'\n',idx);
	AddCh(buf,0,idx);
	oe->p(buf);
}


typedef struct _rtz14_impl_ {
	OE oe;
	uint address_of_one;
	// TODO(rwz): executeProof instantiates local variables like these below
	// however this should be given to the constructor.
	Rnd rnd;
	CommitmentScheme cs;
	Rnd random_source;
	CircuitVisitor poc;
	CircuitVisitor igv;
	CircuitVisitor emt;
	CircuitVisitor pam;
	CircuitVisitor ptb;
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

static XORcommitResult xor_commit(OE oe, Rnd rnd, CommitmentScheme cs, byte * circuit_string, uint lcircuit_string) {
	XORcommitResult res = oe->getmem(sizeof(*res));
	uint i = 0;
	Data m0cmt = 0, m1cmt = 0;
	res->m0 = oe->getmem(lcircuit_string);
	res->m1 = oe->getmem(lcircuit_string);
	rnd->rand(res->m0,lcircuit_string);
	UserReport(oe,"m0 at random: ");
	print_bit_string(oe,Data_shallow(res->m0,lcircuit_string));
	res->lm0 = lcircuit_string;
	res->lm1 = lcircuit_string;


	for(i = 0; i < lcircuit_string; ++i) {
		res->m1[i] = circuit_string[i] ^ res->m0[i];
	}

	// white box usage of commit we use 64 bits per commitment (sha512 with random value of length 16 bytes)
	res->commitment = oe->getmem(128);
	res->lcommitment = 128;
	m0cmt = cs->commit(Data_shallow(res->m0,res->lm0));
	m1cmt = cs->commit(Data_shallow(res->m1,res->lm1));
	mcpy(m0cmt->data,res->commitment,m0cmt->ldata > 64 ? 64 : m0cmt->ldata);
	mcpy(m1cmt->data,res->commitment+64,m1cmt->ldata > 64 ? 64 : m1cmt->ldata);
	xor_commit_end:
	Data_destroy(oe,&m0cmt);
	Data_destroy(oe,&m1cmt);
	return res;
}



static inline void set_bit(byte * bita, uint idx, byte bit) {
	uint byte_idx = idx/8;
	uint bit_idx = idx-byte_idx*8;
	byte mask =  0x01 << bit_idx;
	if (bit == 1)
		bita[byte_idx] |= mask;
	if (bit == 0)
		bita[byte_idx] &= ~mask;
}

/**
 *
 */
static Data build_delta_string(OE oe, List proof_tasks, XORcommitResult xom, byte and_challenge) {
	Data result = Data_new(oe,(proof_tasks->size()+7)/8);
	uint i =0 ;
	for (i = 0; i < proof_tasks->size();++i) {
		ProofTask pt = proof_tasks->get_element(0);
		uint j = 0;
		byte delta = 0;
		for( j = 0; j < (and_challenge == 0 ? 3 : 2);++j) {
			delta ^= get_bit(xom->m0,pt->indicies[j]);
		}
		set_bit(result->data,i,delta);
	}
	return result;
}


COO_DCL(Rtz14,bool,executeProof, List circuit, byte * witness, char * ip, uint port);

COO_DEF_RET_ARGS(Rtz14, bool, executeProof,
		List circuit; byte * witness; char * ip; uint port;,
		circuit, witness, ip,port) {

	Rtz14Impl impl = (Rtz14Impl)this->impl;
	OE oe = impl->oe;
	List input_gates = 0;
	CircuitVisitor emitter = 0;
	// TODO(rwz): take the igv as constructor argument.
	CircuitVisitor igv = 0;
	EmiterResult emitter_res = 0;
	byte * emitted_circuit = 0;
	CArena conn = 0;
	CircuitVisitor proof_task_builder = 0;
	List proof_tasks = 0;
	CircuitVisitor gpam = 0;
	CircuitVisitor poc = 0;
	CircuitVisitor ogv = 0;
	List output_gates = 0;
	Rnd rnd = 0;
	GPam gpam_res = 0;
	DateTime d = DateTime_New(oe);
	ull start = d->getMilliTime();
	_Bool accept = 0;


	// create and call helper strategies
	// TODO(rwz): strategies should be given as constructor arguments instead
	// of being created here. (for testability and maintainability)

	// default one address is 0, however it can be set during create of
	// RTZ14.
	poc = PatchOneConstants_New(oe,impl->address_of_one);
	if (!poc) return False;

	// patch addresses.
	poc->visit(circuit);

	// compute map mapping addresses to input
	igv = InputGateVisitor_New(oe);
	if (!igv) return False;

	input_gates = igv->visit(circuit);
	if (!input_gates) return False;

	ogv = OutputGateVisitor_New(oe);
	if (!ogv) return False;

	output_gates = ogv->visit(circuit);
	if (!output_gates) {
		return False;
	}

	if (output_gates->size() != 1) {
		oe->syslog(OSAL_LOGLEVEL_FATAL,"The provided circuit does not have one unique output.");
		return False;
	}

	OutputGateVisitor_Destroy(&ogv);

	// go online
	conn = CArena_new(oe);
	if (!conn) return False;


	if (ip == 0) {
		// -----------------------------
		// ----------- Prover ----------
		// -----------------------------
		MpcPeer verifier = 0;
		byte and_challenge[1] = {0};
		Data challenge_commitment = Data_new(oe,64);
		XORcommitResult xom = 0;
		Data epsilon = Data_new(oe,1);
		Data delta = 0;
		Data judgement = Data_new(oe,8);
		// The message containing the evaluated circuit, input bits (witness)
		// and the auxiliary informations for each and-gate.
		Data message = 0;

		if (witness == 0) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"No witness !");
			return False;
		}

		UserReport(oe,"Prover preparing ... ");
		// compute evaluated circuit
		emitter = EvaluationStringEmitter_New(oe, input_gates, witness);
		if (!emitter) return False;

		emitter_res = emitter->visit(circuit);
		if (!emitter_res) return False;

		// generate random permutations and majority tests
		rnd = LibcWeakRandomSource_New(oe);
		if (!rnd) return False;

		gpam = GeneratePermuationsAndMajorities_New(oe,rnd,emitter_res->emitted_string);
		if (!gpam) return False;

		gpam_res = gpam->visit(circuit);
		if (!gpam_res) return False;

		// create message
		{
			uint message_size = 0;
			uint idx = 0;
			// the message size in bits is: |circuit| + |inputs| + |aux|
			// where |aux| = #AND_GATES*3
			message_size += emitter_res->lemitted_string;
			message_size += (gpam_res->no_ands*3+7)/8; // three bits aux-info per and gate.

			message = Data_new(oe,message_size);
			UserReport(oe,"[prover] entire message that we commit to:");
			mcpy(message->data+idx,emitter_res->emitted_string,emitter_res->lemitted_string);
			idx+=emitter_res->lemitted_string;
			mcpy(message->data+idx,gpam_res->aux,(gpam_res->no_ands*3+7)/8);
			print_bit_string(oe,Data_shallow(emitter_res->emitted_string,emitter_res->lemitted_string));
			print_bit_string(oe,Data_shallow(gpam_res->aux, (gpam_res->no_ands*3+7)/8));
			print_bit_string(oe,message);

		}

		// inform the user what is happening
		UserReport(oe,"[%lums] Prover is Online and ready, awaiting Verifier to connect ... ",d->getMilliTime()-start);
		conn->listen_wait(1,port);
		oe->p("Verifier connected ...  ");
		verifier = conn->get_peer(0);
		if (verifier == 0) {
			oe->syslog(OSAL_LOGLEVEL_FATAL,"Establishing listening socket or connection failed upon connection. Please check the given port is free and try again. ");
			return False;
		}

		// ---------------------------------------------
		// The Rtz14 protocol starts here for the prover
		// ---------------------------------------------
		UserReport(oe, "Starting proof");
		start = d->getMilliTime();

		// receive commitment to challenge from verifier.
		verifier->receive(challenge_commitment);
		oe->p("Verifier is committed to his challenges");
		UserReport(oe,"[%lums] %s",d->getMilliTime()-start,"Verifier has committed to his challenge");

		// build xor commit to the emitted string
		xom = xor_commit(oe,impl->rnd,impl->cs,message->data, message->ldata);
		UserReport(oe,"[%lums] %s",d->getMilliTime()-start,"[Prover] XOR Commitment prepared");
		print_bit_string(oe,Data_shallow(xom->m0,xom->lm0));
		print_bit_string(oe,Data_shallow(xom->m1,xom->lm1));

		// send the emitted string
		verifier->send(Data_shallow(xom->commitment,xom->lcommitment));

		// receive and and_challenge b
		verifier->receive(Data_shallow(and_challenge,1));
		UserReport(oe,"[%lums] %s",d->getMilliTime()-start,"Received and challenge");

		// send permutation or majority tests
		if (and_challenge[0] == 0) {
			verifier->send(Data_shallow(gpam_res->permutations,(gpam_res->no_ands*3+7)/8));
			proof_task_builder = ProofTaskBuilder_New(oe,and_challenge[0],gpam_res->permutations,(gpam_res->no_ands*3+7)/8);
		} else {
			verifier->send(Data_shallow(gpam_res->majority,(gpam_res->no_ands*2+7)/8));
			proof_task_builder = ProofTaskBuilder_New(oe,and_challenge[0],gpam_res->majority,(gpam_res->no_ands*2+7)/8);
		}

		// build linear proof tasks
		proof_tasks = proof_task_builder->visit(circuit);
		UserReport(oe,"[%lums] %u %s",d->getMilliTime()-start,proof_tasks->size(),
				      "Proof Tasks computed ...");
		{
			uint i = 0;
			for(i = 0; i < proof_tasks->size();++i) {
				ProofTask_print(oe,proof_tasks->get_element(i));
			}

		}

		// build delta string
		delta = build_delta_string(oe,proof_tasks,xom,and_challenge[0]);
		print_bit_string(oe,delta);

		// send delta string to verifier
		UserReport(oe,"[%lums] Sending delta string (%lu)",d->getMilliTime()-start,delta->ldata);
		verifier->send(delta);
		Data_destroy(oe,&delta);

		// receive epsilon
		UserReport(oe,"[%lums] waiting for challenge (epsilon %u) ... ",d->getMilliTime()-start,epsilon->ldata);
		verifier->receive(epsilon);
		UserReport(oe,"[%lums] %s",d->getMilliTime()-start,"Delta string sent and challenge opened receive.");

		// send the m_epsilon string to the verifier
		if (epsilon->data[0] == 0) {
			UserReport(oe,"Sending m0 = %u",xom->lm0);
			verifier->send(Data_shallow(xom->m0,xom->lm0));
		} else {
			UserReport(oe,"Sending m1 = %u",xom->lm0);
			verifier->send(Data_shallow(xom->m1,xom->lm1));
		}

		// receive the string accept/reject from the verifier
		verifier->receive(judgement);
		UserReport(oe,"[%lums] %s",d->getMilliTime()-start,"Proof complete.");

		// tell result to prover.
		oe->p("The verifier says: ");
		oe->p((char *)judgement->data);


		if (judgement->data[0] == 'a') {
			accept = True;
		} else {
			accept = False;
		}

		//TODO(rwz): clean up
		/*
		Data_destroy(oe,&judgement);
		Data_destroy(oe,&message);
		Circuit_Destroy(oe, &circuit);
		GPam_Destroy(oe,&gpam);
		EvaluationStringEmitter_Destroy(&emitter);
*/
	} else {
		// -----------------------------
		// ---------- Verifier ---------
		// -----------------------------

		// ---- The Rtz protocol starts here for the verifier ---

		MpcPeer prover = 0;
		Data challenge_commitment = 0;
		Data challenge = Data_new(oe,64);
		Data commitment_to_circuit = 0;
		Data and_challenge = Data_new(oe,1);
		Data permajor = 0;
		Data delta = 0;
		Data m_challenge = 0;
		uint i = 0, no_ands = 0;

		// generate random challenges and commit to them
		impl->rnd->rand(challenge->data,challenge->ldata);
		challenge->data[0] = challenge->data[0] % 2;
		challenge_commitment = impl->cs->commit(challenge);

		// generate random and-challenge (permutations or majority tests)
		impl->rnd->rand(and_challenge->data,and_challenge->ldata);
		and_challenge->data[0] = (and_challenge->data[0] % 2);

		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"Verifier connecting to Prover ... ");
		// connect to the prover
		conn->connect(ip,port);
		prover = conn->get_peer(0);

		if (!prover) {
			oe->p("No prover sorry, leaving.");
			return -2;
		}

		// send challenge commitment
		prover->send(challenge_commitment);
		UserReport(oe,"[%lums] %s %u bytes", d->getMilliTime()-start,"Commitment to challenge sent",challenge_commitment->ldata);

		// receive the xor-commitment to the circuit
		commitment_to_circuit = Data_new(oe,128);
		prover->receive(commitment_to_circuit);
		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"Circuit XOM commitments received.");

		// send the and challenge
		prover->send(and_challenge);
		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"And challenge sent");

		// count number of ands to anticipate size permajor
		for(i = 0; i < circuit->size();++i) {
			Gate g = circuit->get_element(i);
			if (g->type == G_AND)
				no_ands ++;
		}

		// allocate permajor
		if (and_challenge->data[0] == 0) {
			// permutations
			permajor = Data_new(oe,(3*no_ands+7)/8);
		} else {
			// majority
			permajor = Data_new(oe,(2*no_ands+7)/8);
		}

		// receive permutation or majority tests
		prover->receive(permajor);
		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"Received permutations/majority tests");

		// compute proof tasks the prover must do
		proof_task_builder = ProofTaskBuilder_New(oe,and_challenge->data[0],permajor->data,permajor->ldata);
		proof_tasks = proof_task_builder->visit(circuit);
		UserReport(oe,"[%lums] %u %s", d->getMilliTime()-start,proof_tasks->size(),
				      "Proof Tasks built...");

		// --- Do linear proofs ---

		// --- Receive Delta ---
		delta = Data_new(oe,(proof_tasks->size()+7)/8);
		UserReport(oe, "[%lums] %s delta->size %lu", d->getMilliTime()-start,"Waiting for delta string",delta->ldata);
		prover->receive(delta);
		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"Received deltas from Prover...");

		// send the committed to challenge from the beginning
		challenge->ldata =1;
		prover->send(challenge);

		// receive m_{challange}
		m_challenge = Data_new(oe,(input_gates->size() + circuit->size()+no_ands*3+7)/8);
		prover->receive(m_challenge);
		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"Received XOM partial opening, checking linear relations ...");

		UserReport(oe,"%lums] %s",  d->getMilliTime()-start, "Delta:");
		print_bit_string(oe,delta);

		accept = True;
		for(i = 0; i < proof_tasks->size();++i) {
			byte xor = 0;
			uint j = 0;
			ProofTask cur = proof_tasks->get_element(i);
			uint * indicies = (uint *)&cur->indicies;
			uint lindicies = and_challenge->data[0] == 1 ? 2 : 3;
			for(j = 0;j < lindicies;++j) {
				uint index_j = (uint)indicies[j];
				xor ^= get_bit(m_challenge->data,index_j);
			}

			// this is the equality test which is the only thing
			// this implementation needs to far !
			if (xor != (byte)get_bit(delta->data,i)) {
				accept = False;
			}
		}

		if (accept) {
			prover->send(Data_shallow((byte*)"accept ",8));
			oe->p("Verifier accepted proof.");
			accept = True;
		} else {
			prover->send(Data_shallow((byte*)"reject ",8));
			oe->p("Verifier rejected proof");
			accept = 0;
		}
		UserReport(oe,"[%lums] %s", d->getMilliTime()-start,"Verifier Done.");
	}

	// close all connections.
	CArena_destroy(&conn);

	return accept;
	error:
	return accept;
}



Rtz14 Rtz14_New(OE oe, Rnd rnd, CommitmentScheme scheme, uint address_of_one) {
	// Instance
	Rtz14 res = 0;
	Rtz14Impl impl = 0;

	// input check
	if (!oe) return 0;
	if (!rnd) return 0;
	if (!scheme) return 0;

	res = (Rtz14)oe->getmem(sizeof(*res));
	if (!res) return 0;

	impl = (Rtz14Impl)oe->getmem(sizeof(*impl));
	if (!impl) goto error;

	res->impl = impl;


	// populate impl
	impl->oe = oe;
	impl->rnd = rnd;
	impl->cs = scheme;
	impl->address_of_one = address_of_one;

	// setup interface functions
	COO_ATTACH(Rtz14,res,executeProof);

	return res;
	error:
	Rtz14_Destroy(&res);
	if (res) {
		oe->putmem(res);
	}
	return 0;
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
