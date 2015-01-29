/*
 * emitter.h
 *
 *  Created on: Jan 8, 2015
 *      Author: rwl
 */

#ifndef RTZ14_INCLUDE_EMITTER_H_
#define RTZ14_INCLUDE_EMITTER_H_

#include <osal.h>
#include <singlelinkedlist.h>
#include <map.h>
#include <circuit_analyser.h>

typedef struct _emitter_result_ {
	byte * emitted_string;
	uint lemitted_string;
} * EmiterResult;

// clean up emiter result
void EmiterResult_Destroy(OE oe,EmiterResult * er);

void apply_permutation(byte * perms, uint perm_idx, uint * indices);
void apply_majority(byte * maj, uint maj_idx, uint * indices);

/**
 * EvaluationStringEmitter
 *
 * This emiter builds a string that is the evaluation
 * of a circuit provided to the visit method. The inputs are
 * taken from input_gates. This the circuit with the input_gates map must
 * form a closure with no free heap addresses. When the heap is populated
 * the heap is mapped to a bit-string with groups of three: two input bits
 * and one output bit.
 *
 * \param oe - operating environment
 *
 * \param input_gates - gates that are used for the inputs for the circuit
 *                      provided to the visitor.
 *
 * \return a fresh visitor that emits a byte * from its visit method
 * which is  the evaluation string of the circuit.
 *
 */
CircuitVisitor EvaluationStringEmitter_New(OE oe, Map input_gates, byte * input);

void EvaluationStringEmitter_Destroy(CircuitVisitor * cv);

#endif /* RTZ14_INCLUDE_EMITTER_H_ */
