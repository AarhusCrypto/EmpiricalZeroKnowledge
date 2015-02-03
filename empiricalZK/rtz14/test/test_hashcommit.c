/*
 * test_hashcommit.c
 *
 *  Created on: Feb 2, 2015
 *      Author: rwl
 */

#include <test.h>
#include <testutils.h>
#include <osal.h>
#include <commitment.h>

static int create_instance(OE oe) {
	_Bool ok = 1;
	Rnd rnd = TestRnd_New(oe,0);
	CommitmentScheme cs = Sha512BasedUcScheme_New(oe,rnd);

	AssertTrue( cs != 0)
	Sha512BasedUcScheme_Destroy(&cs);
	AssertTrue( cs == 0)
	test_end:
	return ok;
}


static int test_hash_one_byte(OE oe) {
	_Bool ok = 1;
	Rnd rnd = TestRnd_New(oe,0);
	CommitmentScheme cs = Sha512BasedUcScheme_New(oe,rnd);
	Data cmt = 0;
	byte m[1] = {'r'};

	AssertTrue( cs != 0)
	cmt = cs->commit(Data_shallow(m,1));
	AssertTrue(cs->open(cmt,Data_shallow(m,1)) == True);
	Sha512BasedUcScheme_Destroy(&cs);
	AssertTrue(cs == 0)

	test_end:
	Data_destroy(oe,&cmt);
	return ok;
}


static int test_hash_byte_array(OE oe) {
	_Bool ok = 1;
	Rnd rnd = TestRnd_New(oe,0);
	CommitmentScheme cs = Sha512BasedUcScheme_New(oe,rnd);
	byte m[] = {"Once upon a time there was a message."};
	Data cmt = 0;

	cmt = cs->commit(Data_shallow(m,sizeof(m)));
	AssertTrue(cs->open(cmt,Data_shallow(m,sizeof(m))) == True);
	Sha512BasedUcScheme_Destroy(&cs);
	AssertTrue(cs == 0)

	test_end:
	return ok;
}

static int test_wrong_message(OE oe) {
	_Bool ok = 1;
	Rnd rnd = TestRnd_New(oe,0);
	CommitmentScheme cs = Sha512BasedUcScheme_New(oe,rnd);
	byte m[] = {'a'};
	byte m1[] = {'b'};
	Data cmt = 0;

	cmt = cs->commit(Data_shallow(m,1));
	AssertTrue ( cs->open(cmt,Data_shallow(m1,1)) == False);

	test_end:
	return ok;
}

Test tests[] = {
		{"creating an instance",create_instance},
		{"hashing one byte", test_hash_one_byte},
		{"hashing byte array", test_hash_byte_array},
		{"fail on wrong message", test_wrong_message}
};

TEST_MAIN
