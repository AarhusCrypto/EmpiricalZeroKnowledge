/*
 * linear_proof.h
 *
 *  Created on: Jan 9, 2015
 *      Author: rwl
 */

#ifndef RTZ14_INCLUDE_LINEAR_PROOF_H_
#define RTZ14_INCLUDE_LINEAR_PROOF_H_

#include <singlelinkedlist.h>
#include <osal.h>
#include <circuit_analyser.h>

typedef struct _proof_task_ {
	uint indicies[3];
	byte value;
} * ProofTask;


/**
 * Create a fresh proof task builder. Its visit method will return a list of ProofTask's
 * that shall be used in the Rtz14 protocol. These proofs are the relations that needs to
 * be checked for and gates and xor-gates.
 *
 * \param oe - the operating environment
 *
 * \param and_challenge - 0 if permutations should be proven; 1 for majority.
 * TODO(rwz): write doc
 * \return a circuit vistor that will visit the circuit and return a list of proof tasks from
 * its visit method.
 *
 */
CircuitVisitor ProofTaskBuilder_New(OE oe, byte and_challenge, byte * permaj, uint lpermaj);

// clean up the proof task builder.
void ProofTaskBuilder_Destroy(CircuitVisitor * ptb);

#endif /* RTZ14_INCLUDE_LINEAR_PROOF_H_ */
