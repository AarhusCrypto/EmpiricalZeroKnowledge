/*
 * commitment.h
 *
 *  Created on: Jan 22, 2015
 *      Author: rwl
 */

#ifndef RTZ14_INCLUDE_COMMITMENT_H_
#define RTZ14_INCLUDE_COMMITMENT_H_

#include <common.h>
#include <osal.h>
#include <rnd.h>

/**
 * The type of a commitment scheme.
 *
 */
typedef struct _commitment_scheme_ {
	/**
	 *
	 * The commit function create the data that is a commitment,
	 * a data array.
	 *
	 * \param message - the message to commit too.
	 *
	 * \return data that is the commitment, hiding and binding.
	 */
	Data (*commit)(Data message);

	/**
	 * The open function takes a commitment and a message and checks that
	 * the given commitment is to that message.
	 *
	 * \param
	 */
	bool (*open)(Data commitment,Data message);

	void * impl;
} * CommitmentScheme;

/**
 * Create a dummy scheme for testing purposes, basically the identity function.
 */
CommitmentScheme DummyScheme_New(OE oe);

// clean up dummy scheme
void DymmyScheme_Destroy(CommitmentScheme * scheme);

/**
 * Create Sha512 based Uc scheme, security relies on the random source too.
 */
CommitmentScheme Sha512BasedUcScheme_New(OE oe, Rnd rnd);

// clean up Sha512BasedUcScheme !
void Sha512BasedUcScheme_Destroy(CommitmentScheme * scheme);

#endif /* RTZ14_INCLUDE_COMMITMENT_H_ */
