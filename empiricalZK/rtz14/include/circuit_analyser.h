/*
 * circuit_analyser.h
 *
 *  Created on: Jan 7, 2015
 *      Author: rwl
 */

#ifndef RTZ14_INCLUDE_CIRCUIT_ANALYSER_H_
#define RTZ14_INCLUDE_CIRCUIT_ANALYSER_H_

#include "circuitparser.h"

typedef struct _circuit_visitor_ {
	void * (*visitAnd)(Gate and);
	void * (*visitXor)(Gate xor);
	void * (*visit)(List circuit);
	void * impl;
} * CircuitVisitor;

/**
 * Inversion gates are not supported and our cuitcuit definitions requires that
 * one particular location in the heap is loaded with the public constant ONE.
 *
 * \param oe - os abstraction.
 *
 * \param circuit - the list of all parsed gates.
 *
 * \param address_of_one - the address of where the public ONE constant is loaded.
 */
CircuitVisitor PatchOneConstants_New(OE oe, uint address_of_one);

// clean up PatchOneConstants visitor
void PatchOneConstants_Destroy(CircuitVisitor * pcv_);

/**
 * The input gate visitor computes all heap references
 * that are used before written to. These gates are
 * considered input gates as not providing a value for these
 * heap address locations would exhibit implementation defined
 * behavior (undefined, that is).
 */
CircuitVisitor InputGateVisitor_New(OE oe);

// clean up the input gate visitor
void InputGateVisitor_Destroy(CircuitVisitor * instance);

/**
 * Compute all gates that are never read and we assume these are
 * the output gates.
 *
 */
CircuitVisitor OutputGateVisitor_New(OE oe);

// clean up the output gate visitor
void OutputGateVisitor_Destroy(CircuitVisitor * cv);

#endif /* RTZ14_INCLUDE_CIRCUIT_ANALYSER_H_ */
