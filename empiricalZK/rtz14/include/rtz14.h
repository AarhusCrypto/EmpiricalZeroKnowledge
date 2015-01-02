#ifndef RTZ14_H
#define RTZ14_H

#include <common.h>
#include <osal.h>
#include <common.h>

typedef struct _rtz14_ {

	/**
	 * \briefs return true of this instance is a prover. Eg. there exists a witness.
	 */
	bool (*isProver)();


	/**
	 *
	 * \brief Execute the proof as prover listening for clients if {proverIP} is (void*)0; otherwise
	 * connect to {proverIP} on port {port} expecting to run as verifier with a prover there.
	 *
	 * Notice the {witness} in the constructor cannot be (void*)0 AKA NULL if no {proverIP}
	 * argument is given.
	 *
	 * \return true if the proof was successful and the verifier accepted.
	 *
	 */
	bool (*executeProof)(char * proverIP, uint port);

	// The implementation details hidden as a mem blob.
	void * impl;

} * Rtz14;

/**
 *
 * Create a fresh RTZ ZeroKnowledge instance
 *
 * \param oe     - The operating system abstraction
 *
 * \param circuit_file - File containing description of the circuit
 *
 * \param witness - the witness encoded as eight bit per byte can be omitted
 *                  if this instance is expected to run a verifier.
 *
 * \return a fresh zero knowledge instance.
 */
Rtz14 Rtz14_New(OE oe, char * circuit_file, byte * witness);

/**
 *
 * Free the resources used by {instance}.
 *
 */
void Rtz14_Destroy(Rtz14 * instance);

#endif
