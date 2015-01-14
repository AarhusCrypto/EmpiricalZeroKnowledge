#include "test.h"
#include <osal.h>
#include <linear_proof.h>

static int test_create_linear_prover(OE oe) {
	_Bool ok = 0;
	CircuitVisitor ptb = ProofTaskBuilder_New(oe, 0,0,0);

	ok = (ptb != 0);
	ProofTaskBuilder_Destroy(&ptb);

	return ok;
}

Test tests[] = {
		{"creating a linear prover.", test_create_linear_prover}
};

TEST_MAIN
