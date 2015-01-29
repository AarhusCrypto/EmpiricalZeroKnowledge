#ifndef RTZ14_H
#define RTZ14_H

#include <common.h>
#include <osal.h>
#include <singlelinkedlist.h>
#include <common.h>
#include <rnd.h>
#include <commitment.h>

/*
 * Macro for writing nice messages to the user.
 */
#define UserReport(oe,msg,...) {\
	byte ____b[512] = {0}; \
	osal_sprintf((char*)____b,(msg),##__VA_ARGS__);\
	(oe)->syslog(OSAL_LOGLEVEL_USER,____b); }


typedef struct _rtz14_ {

	/**
	 *
	 * \brief Execute the proof as prover listening for clients if {proverIP} is (void*)0; otherwise
	 * connect to {proverIP} on port {port} expecting to run as verifier with a prover there.
	 *
	 * If run without {proverIP} as prover the {witness} shall be supplied and satisfy the circuit.
	 *
	 * \param circuit - the gates of the circuit to prove sat for
	 *
	 * \param witness - the inputs that satisfy the circuit
	 *
	 * \param proverIP - the ip address of the prover
	 *
	 * \param port - the port of the prover
	 *
	 * \return true if the proof was successful and the verifier accepted.
	 *
	 * Example:
	 *
	 *  Rtz14 zk = Rtz14_New(oe);
	 *  Tokenizer tk = <some tokenizer>();
	 *  CircuitParser cp = CircuitParser_New(oe,tk);
	 *  List circuit = 0;
	 *
	 *  circuit = cp->parseSource(buffer,lbuffer);
	 *  #ifdef IS_PROVER
	 *  zk->executeProof(circuit,witness,0,2020);
	 *  #else
	 *  zk->executeProof(circuit,0,"127.0.0.1",2020);
	 *  #endif
	 *
	 */
	bool (*executeProof)(List circuit, byte * witness, char * proverIP, uint port);

	// The implementation details hidden as a mem blob.
	void * impl;

} * Rtz14;

/**
 *
 * Create a fresh RTZ ZeroKnowledge instance
 *
 * \param oe     - The operating system abstraction.
 *
 * \param rnd    - random source to use.
 *
 * \param scheme - underlying commitment scheme to use
 *
 * \param address_of_one - the heap address that yields
 *                         the constant one initially.
 *
 * \return a fresh zero knowledge instance.
 */
Rtz14 Rtz14_New(OE oe, Rnd rnd, CommitmentScheme scheme, uint address_of_one);

/**
 *
 * Free the resources used by {instance}.
 *
 */
void Rtz14_Destroy(Rtz14 * instance);

#endif
