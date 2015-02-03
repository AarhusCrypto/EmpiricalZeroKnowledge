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
#include <rnd.h>

/**
 * A {ProofTask} is a set of linear relations that can be proven
 * using the RTZ14 protocol. In the paper denoted $$ (I,b) $$ where I
 * is a set of indices into our string M that xor'ed together should yield b.
 *
 * There is no limit to how many indices I may hold in theory however for
 * all practical purposes we need 3.
 *
 * Given permutations/majority tests encoded as bits this module generate {ProofTask}s.
 */
typedef struct _proof_task_ {
	uint indicies[3];
	// value can be either 0 or 1 which we will encode in the least significant bit
	// the next two bits indicate the length of indicies. e.g. value/2 == lengthof(indicies);
	byte value;
} * ProofTask;

/**
 * Print proof tasks to the log on level debug.
 */
void ProofTask_print(OE oe, ProofTask pt);

/**
 * proof tasks are created as a list by the visit method of the proof task builder. For
 * convenience we provide a way to clean that up easily.
 *
 * \param oe - Environment
 *
 * \param pt - list of proof tasks to clean up.
 *
 *
 */
void ProofTasks_Destroy(OE oe, List * pt) ;

/**
 * Create a fresh proof task builder. It's visit method will return a list of ProofTasks
 * that may be used in the Rtz14 protocol. These proofs are the relations that needs to
 * be checked for and gates and xor-gates.
 *
 * \param oe - the operating environment
 *
 * \param and_challenge - 0 if permutations should be proven; 1 for majority.
 *
 * \param permaj - bit array; if {and_challenge} 0 bits are grouped in three indicating a number
 * between 0 and 5 (both inclusive) pointing out a permutation of three elements. There shall be
 * one triple of bits per and gate in the circuit provided for {visit}. if {and_challenge} is 1
 * bits are grouped in two indicating a majority permutation between 0 and 2. There shall be one
 * group of two bits per and gate provided for {visit}.
 *
 * \param lpermaj - the length for permaj.
 *
 * \param no_inputs - the number of input gates
 *
 * \return a circuit visitor that will visit the circuit and return a list of proof tasks from
 * its visit method.
 *
 */
CircuitVisitor ProofTaskBuilder_New(OE oe, byte and_challenge, byte * permaj, uint lpermaj, uint no_inputs);

// clean up the proof task builder.
void ProofTaskBuilder_Destroy(CircuitVisitor * ptb);

/**
 * In the protocol the verifier will either ask the prover to give majority permutations or random
 * permutations. A honest prover will need to generate both kinds of permutations consistent with the
 * circuit evaluation.
 *
 * The prover will use this visitor to generate such permutations. The prover then uses these
 * to be put into the committed string representing the auxiliary info appended to the circuit
 * evaluation.
 *
 * Then the verifier will ask for either of the strings and thereby prover and verifier can use the
 * proofTaskBuilder to build an appropriate set of linear proof tasks that will convincingly prove
 * to the verifier that the witness is known to the prover.
 *
 * \param oe - operating environment
 *
 * \param rnd - a random source
 *
 * \param evaled_circuit - the evaluated circuit
 *
 * \return a fresh circuit visitor for generating random permutations consistent with the
 * circuit given to visit.
 *
 */
CircuitVisitor GeneratePermuationsAndMajorities_New(OE oe, Rnd rnd, byte * evaled_circuit);

/**
 * The result from running the {GeneratePermutationsAndMajorities} visitor on
 * a circuit. The visit method returns {GPam}.
 */
typedef struct _gpam_result_ {
	/**
	 * {permutations} is a bit-array containing groups of three bits each an index
	 * pointing out one permutations of three elements as a number (0-5).
	 *
	 * {permutations} will have byte-length (#AND_GATES*3+7)/8 e.g.
	 * ceil(#AND_GATES*3/8) elements.
	 */
	byte * permutations;
	/**
	 * {majority} is a bit-array containing pairs of bits each an index pointing out
	 * a permutation of two of the bits (in a group of three) such that the first two
	 * bits are the majority of three bits. The index is from 0-2.
	 *
	 * {majority} will have byte-length ceil(#AND_GATES*2/8)
	 */
	byte * majority;

	/**
	 * {aux} is the auxiliary data that needs to be appended to the circuit evaluation
	 * string and committed to in the RTZ14 protocol.
	 *
	 * {aux} will have byte-length ceil(#AND_GATES*3/8).
	 */
	byte * aux;

	/**
	 * Number of and gates (e.g. #AND_GATES).
	 */
	uint no_ands;
} *GPam;

/// Destroy/cleanup the GPam, note the same OE shall be given to {GPam_Destroy} as was given to
/// {GeneratePermutationsAndMajorities_New}.
void GPam_Destroy(OE oe, GPam * gpam);

// Clean up {GenratePermuataionsAndMajorities} visitor
void GeneratePermutationsAndMajorities_Destroy(CircuitVisitor * cv);

#endif /* RTZ14_INCLUDE_LINEAR_PROOF_H_ */
