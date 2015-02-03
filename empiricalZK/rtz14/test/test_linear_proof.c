#include "test.h"
#include <osal.h>
#include <linear_proof.h>
#include <coov3.h>
#include <rnd.h>
#include <map.h>
#include <emiter.h>
#include "testutils.h"

static int test_create_linear_prover(OE oe) {
	_Bool ok = 0;
	CircuitVisitor ptb = ProofTaskBuilder_New(oe, 0,0,0,0);

	ok = (ptb != 0);
	ProofTaskBuilder_Destroy(&ptb);
	ok = (ptb == 0);

	return ok;
}

/**
 * Test Helper to build a circuit from string source quickly.
 *
 */
static List build_circuit(OE oe, char * cstr) {
	Tokenizer tk = FunCallTokenizer_New(oe);
	CircuitParser cp = CircuitParser_New(oe,tk);

	List circuit = 0;
	uint lcstr = 0;
	while(cstr[lcstr]) ++lcstr;
	circuit = cp->parseSource((byte*)cstr,lcstr);

	CircuitParser_Destroy(&cp);
	FunCallTokenizer_Destroy(&tk);

	return circuit;
}


/*
 * Test generation of permutation with one and gate.
 */
static int test_one_and_gate_permutation(OE oe) {
	_Bool ok = 1;
	byte perms[1] = {0}; // identity permutation
	CircuitVisitor ptb = ProofTaskBuilder_New(oe,0 /*do permutations*/,perms,1,2);
	List circuit = build_circuit(oe,"AND(0,1,2)");
	List proofTasks = 0;
	ProofTask pt = 0;
	proofTasks = ptb->visit(circuit);

	AssertTrue(proofTasks != 0);
	AssertTrue(proofTasks->size() == 3);

	// There is three prooftasks in the result
	// and these must have indicies corresponding to laying out a string
	// as follows: m_0 m_1 m_2 p_0 p_1 p_2 where m_0 := (m_1 logical_and m_2)
	// the permutations we insert here yields the identity so we need to see
	// eq1->indicies[0] = 0 and eq1->indicies[1] = circuitsize+2 + identity_perm(0,1,2)[0] = 3
	// eq2->indicies[0] = 1 and eq2->indicies[1] = circuitsize+2 + identity_perm(0,1,2)[1] = 4
	// eq3->indicies[0] = 2 and eq3->indicies[1] = circuitsize+2 + identity_perm(0,1,2)[2] = 5

	pt = proofTasks->get_element(0);
	AssertTrue(pt->indicies[0] == 0)
	AssertTrue(pt->indicies[1] == 3);
	AssertTrue(pt->value == 4);

	pt = proofTasks->get_element(1);
	AssertTrue(pt->indicies[0] == 1);
	AssertTrue(pt->indicies[1] == 4);
	AssertTrue(pt->value == 4)

	pt = proofTasks->get_element(2);
	AssertTrue(pt->indicies[0] == 2);
	AssertTrue(pt->indicies[1] == 5);
	AssertTrue(pt->value == 4)

test_end:
	ProofTaskBuilder_Destroy(&ptb);
	Circuit_Destroy(oe,&circuit);
	ProofTasks_Destroy(oe,&proofTasks);
	return ok;
}


static int test_one_and_gate_majority(OE oe) {
	_Bool ok = 1;
	byte maj[1] = {0}; // identity permutation
	CircuitVisitor ptb = ProofTaskBuilder_New(oe,1 /*do majority*/,maj,1,2);
	List circuit = build_circuit(oe,"AND(0,1,2);");
	List proofTasks = 0;
	ProofTask pt = 0;
	proofTasks = ptb->visit(circuit);

	AssertTrue( proofTasks != 0)

	AssertTrue( proofTasks->size() == 2)

	pt = proofTasks->get_element(0);
	AssertTrue( pt != 0 )
	AssertTrue( pt->indicies[0] == 1)
	AssertTrue( pt->indicies[1] == 3)
	AssertTrue( pt->value == 4)

	pt = proofTasks->get_element(1);
	AssertTrue( pt != 0 )
	AssertTrue( pt->indicies[0] == 2)
	AssertTrue( pt->indicies[1] == 4)
	AssertTrue( pt->value == 4)


test_end:
    ProofTaskBuilder_Destroy(&ptb);
    Circuit_Destroy(oe, &circuit);
    ProofTasks_Destroy(oe, &proofTasks);
    return ok;
}

static int test_one_xor_gate_permutation(OE oe) {
	_Bool ok = 1;

	return ok;
}

static int test_one_xor_gate_majority(OE oe) {
	_Bool ok = 1;

	return ok;
}


static int test_multi_and_gates(OE oe) {
	_Bool ok = 1;

	return ok;
}

static int test_multi_xor_gates(OE oe) {
	_Bool ok = 1;

	return ok;
}

static int test_one_of_each(OE oe) {
	_Bool ok = 1;

	return ok;
}

static int test_small_circuit(OE oe) {
	_Bool ok = 1;

	return ok;
}

static inline byte get_bit(byte * bita, uint idx) {
	uint byte_idx = idx/8;
	uint bit_idx = idx-byte_idx*8;
	byte mask =  0x01 << bit_idx;
	return (bita[byte_idx] & mask) != 0;
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

static int test_get_bit(OE oe) {
	_Bool ok = 1;
	byte a[3] = {7,1,129};

	AssertTrue( get_bit(a,0) == 1);
	AssertTrue( get_bit(a,1) == 1)
	AssertTrue( get_bit(a,2) == 1)
	AssertTrue( get_bit(a,8) == 1);
	AssertTrue( get_bit(a,16) == 1);
	AssertTrue( get_bit(a,23) == 1)

	test_end:
	return ok;
}

static int test_set_bit(OE oe) {
	_Bool ok = 1;
	//             00001010,00000101,11111111
	byte data[] = {  0x0A  ,  0x05  ,  0xFF  };
	set_bit(data,0,1);
	set_bit(data,9,1);
	set_bit(data,23,0);
	// expected 00001011,00000101
	AssertTrue(data[0] == 0x0B);
	AssertTrue(data[1] == 0x07);
	AssertTrue(data[2] == 0x7F)

	test_end:
	return ok;
}


// test that we can create an instance of generate perm and maj visitor
static int test_generate_perm_and_maj_create(OE oe) {
	_Bool ok = 1;
	byte d[] = {0};
	Rnd rnd = TestRnd_New(oe,0);
	CircuitVisitor cv = GeneratePermuationsAndMajorities_New(oe,rnd,d);

	AssertTrue(cv != 0)
	test_end:
	return ok;
}

// test the generate perm and maj visitor is correct in the all zero case
static int test_permmaj_one_allzero_idperm_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {0};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	Rnd rnd = TestRnd_New(oe,0);

	circuit = build_circuit(oe,"// Address 3 will contain 1\n AND(0,1,2");
	pog = PatchOneConstants_New(oe,3);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	AssertTrue(gpam_result->permutations[0] == 0)
	AssertTrue(gpam_result->majority[0] == 0)
	AssertTrue(gpam_result->aux[0] == 0)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}


// test the perm and maj visitor is correct in the all ones case
// with identity permutations
static int test_permmaj_one_op1one_op2one_idperm_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {1};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	uint permutation_to_use = 0;
	// the random choice coming out of the rnd determines the permuation to use
	Rnd rnd = TestRnd_New(oe,permutation_to_use);

	// so we fix address of 1 to 0
	// we invert addresses 1 and 2 meaning we have ones there
	// Then we will get a one in address 3 yielding the bit string
	// ADR: 0 1 2   3
	//      1 1 1   1 = 15
	circuit = build_circuit(oe,"//INV(1,1);INV(2,2);AND(3,1,2)");
	pog = PatchOneConstants_New(oe,0);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(er->emitted_string[0] == 15)
	AssertTrue(circuit->size() > 1)
	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	// we use the identity permutation for the and gate AND(3,1,2)
	// so it should be 0
	AssertTrue(gpam_result->permutations[0] == 0)
	// we use the identity permutation and because we have all ones
	// the identity is also good for the majority permutation. !
	AssertTrue(gpam_result->majority[0] == 0)
	// the auxiliary data should be 111=7
	AssertTrue(gpam_result->aux[0] == 7)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}


static int test_permmaj_one_op1one_op2zero_perm2_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {1};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	uint permutation_to_use = 2;
	// the random choice coming out of the rnd determines the permuation to use
	Rnd rnd = TestRnd_New(oe,permutation_to_use);

	// so we fix address of 1 to 0
	// we invert addresses 1 meaning we have one as first operand to AND
	// Then we will get a zero in address 3 yielding the bit string
	//
	// pi={1,0,2} betyder position 0 og 1 skifter plads, position 0 og et er
	// 0 er op1 og position 1 er op2 og position 3 er dst. Derfor har vi at
	// pi(010)=pi(bit3bit1bit2)=>(bit3bit2bit1)
	//      c1 b1 b2  b3
	// ADR: 0  1  2   3     pi(dst,op1,op2) => (op1,dst,op2)
	//      1  1  0   0 = 3 pi(010)=(001)
	circuit = build_circuit(oe,"//INV(1,1);AND(3,1,2)");
	pog = PatchOneConstants_New(oe,0);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(er->emitted_string[0] == 3)
	AssertTrue(circuit->size() > 1)
	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	AssertTrue(gpam_result->permutations[0] == 2)
	AssertTrue(gpam_result->majority[0] == 2)
	// the auxiliary data should be 001=1
	AssertTrue(gpam_result->aux[0] == 1)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}


static int test_permmaj_one_op1one_op2zero_perm3_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {1};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	uint permutation_to_use = 3;
	// the random choice coming out of the rnd determines the permuation to use
	Rnd rnd = TestRnd_New(oe,permutation_to_use);

	// so we fix address of 1 to 0
	// we invert addresses 1 meaning we have one as first operand to AND
	// Then we will get a zero in address 3 yielding the bit string
	// ADR: 0 1 2   3
	//      1 1 0   0 = 3
	// pi = {1,2,0}, pi(dst,op1,op2) = (op1,op2,dst) = (100)
	//
	circuit = build_circuit(oe,"//INV(1,1);AND(3,1,2)");
	pog = PatchOneConstants_New(oe,0);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(er->emitted_string[0] == 3)
	AssertTrue(circuit->size() > 1)
	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	// we use the identity permutation for the and gate AND(3,1,2)
	// so it should be 0
	AssertTrue(gpam_result->permutations[0] == 3)
	// we use the identity permutation and because we have all ones
	// the identity is also good for the majority permutation. !
	AssertTrue(gpam_result->majority[0] == 0)
	// the auxiliary data should be 100=4
	AssertTrue(gpam_result->aux[0] == 4)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}


static int test_permmaj_one_op1one_op2zero_perm4_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {1};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	uint permutation_to_use = 4;
	// the random choice coming out of the rnd determines the permuation to use
	Rnd rnd = TestRnd_New(oe,permutation_to_use);

	// so we fix address of 1 to 0
	// we invert addresses 1 meaning we have one as first operand to AND
	// Then we will get a zero in address 3 yielding the bit string
	// ADR: 0 1 2   3
	//      1 1 0   0 = 3
	// pi = {2,0,1} pi(dst,op1,op2) = (op2,dst,op1) = (001)
	circuit = build_circuit(oe,"//INV(1,1);AND(3,1,2)");
	pog = PatchOneConstants_New(oe,0);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(er->emitted_string[0] == 3)
	AssertTrue(circuit->size() > 1)
	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	AssertTrue(gpam_result->permutations[0] == 4)
	AssertTrue(gpam_result->majority[0] == 2)
	// the auxiliary data should be
	AssertTrue(gpam_result->aux[0] == 1)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}


static int test_permmaj_one_op1one_op2zero_perm5_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {1};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	uint permutation_to_use = 5;
	// the random choice coming out of the rnd determines the permuation to use
	Rnd rnd = TestRnd_New(oe,permutation_to_use);

	// so we fix address of 1 to 0
	// we invert addresses 1 meaning we have one as first operand to AND
	// Then we will get a zero in address 3 yielding the bit string
	// ADR: 0 1 2   3
	//      1 1 0   0 = 3
	// pi = {2,1,0} pi(dst,op1,op2) = (op2,op1,dst) = (010)
	circuit = build_circuit(oe,"//INV(1,1);AND(3,1,2)");
	pog = PatchOneConstants_New(oe,0);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(er->emitted_string[0] == 3)
	AssertTrue(circuit->size() > 1)
	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	// we use the identity permutation for the and gate AND(3,1,2)
	// so it should be 0
	AssertTrue(gpam_result->permutations[0] == 5)
	AssertTrue(gpam_result->majority[0] == 1)
	AssertTrue(gpam_result->aux[0] == 2)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}


static int test_permmaj_one_op1one_op2zero_perm1_and_gate(OE oe) {
	_Bool ok = 1;
	byte inputs[1] = {1};
	byte * evaled_circuit = 0;
	Tokenizer tk = 0;
	List circuit = 0;
	Map input_gates = 0;
	CircuitVisitor pog = 0;
	CircuitVisitor igv = 0;
	CircuitVisitor emiter = 0;
	CircuitVisitor gpam = 0;
	EmiterResult er = 0;
	GPam gpam_result = 0;
	uint permutation_to_use = 1;
	// the random choice coming out of the rnd determines the permuation to use
	Rnd rnd = TestRnd_New(oe,permutation_to_use);

	// so we fix address of 1 to 0
	// we invert addresses 1 meaning we have one as first operand to AND
	// Then we will get a zero in address 3 yielding the bit string
	// ADR: 0 1 2   3
	//      1 1 0   0 = 3 pi(010) = (001)
	circuit = build_circuit(oe,"//INV(1,1);AND(3,1,2)");
	pog = PatchOneConstants_New(oe,0);
	pog->visit(circuit);
	igv = InputGateVisitor_New(oe);
	input_gates = igv->visit(circuit);
	emiter = EvaluationStringEmitter_New(oe,input_gates,inputs);
	er = emiter->visit(circuit);

	gpam = GeneratePermuationsAndMajorities_New(oe,rnd,er->emitted_string);
	gpam_result = gpam->visit(circuit);

	AssertTrue(er->emitted_string[0] == 3)
	AssertTrue(circuit->size() > 1)
	AssertTrue(gpam_result != 0)
	AssertTrue(gpam_result->aux != 0)
	AssertTrue(gpam_result->permutations != 0)
	AssertTrue(gpam_result->majority != 0)
	// we use the identity permutation for the and gate AND(3,1,2)
	// so it should be 0
	AssertTrue(gpam_result->permutations[0] == 1)
	// we use the identity permutation and because we have all ones
	// the identity is also good for the majority permutation. !
	AssertTrue(gpam_result->majority[0] == 0)
	// the auxiliary data should be 100=4
	AssertTrue(gpam_result->aux[0] == 4)
test_end:
	Circuit_Destroy(oe,&circuit);
	PatchOneConstants_Destroy(&pog);
	InputGateVisitor_Destroy(&igv);
	EvaluationStringEmitter_Destroy(&emiter);
	EmiterResult_Destroy(oe,&er);

	return ok;
}




Test tests[] = {
		{"creating a linear prover.", test_create_linear_prover},
		{"one and gate permutation",test_one_and_gate_permutation},
		{"one and gate majority", test_one_and_gate_majority},
		{"one xor gate permutation", test_one_xor_gate_permutation},
		{"one xor gate majority", test_one_xor_gate_majority},
		{"multiple and gates", test_multi_and_gates},
		{"multiple xor gates", test_multi_xor_gates},
		{"combined one xor and one and-gate", test_one_of_each},
		{"small circuit",test_small_circuit},
		{"Test set bit", test_set_bit},
		{"Test get bit", test_get_bit},
		{"test generate perm/maj create", test_generate_perm_and_maj_create},
		{"generate perm/maj one and gate id perm", test_permmaj_one_allzero_idperm_and_gate},
		{"generate perm/maj one and gate id perm", test_permmaj_one_op1one_op2one_idperm_and_gate},
		{"generate perm/maj one and gate perm1  ", test_permmaj_one_op1one_op2zero_perm1_and_gate},
		{"generate perm/maj one and gate perm2  ", test_permmaj_one_op1one_op2zero_perm2_and_gate},
		{"generate perm/maj one and gate perm3  ", test_permmaj_one_op1one_op2zero_perm3_and_gate},
		{"generate perm/maj one and gate perm4  ", test_permmaj_one_op1one_op2zero_perm4_and_gate},
		{"generate perm/maj one and gate perm5  ", test_permmaj_one_op1one_op2zero_perm5_and_gate},
		// TODO(rwz): more tests would be nice. E.g.:
		//  I] test other AND-gate assignments
		// II] tests with larger circuits e.g. more than one and gate.
		//III] proof task builder actually builds the right tasks given
		//     the output from the evaluation emitter.

};

TEST_MAIN
