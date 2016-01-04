EmpiricalZeroKnowledge
======================

We implement the RTZ14 protocol here using ROM commitments H(x;r) . 

Interesting future work includes:

  * Implementing RTZ14 with the extremely efficient UC-commitments from CDDGNT14. Complexity O(k|C|)
  * implementing IKOS07 with Scalable MPC from ID06 and compare. Complexity O(|C|).

We conjecture that the former construction will be faster for small circuits and reasonble statictical security parameter k.

Currently only RTZ14 is implemented for the publication of the paper https://eprint.iacr.org/2014/934.pdf.

Building the code
=================

With the proper dependencies (automake and autoconf) run

./build.sh release

Running The Code
=================

We have one test circuit for proving knowledge of an AES key enciphering 
the all zero block to its encryption under the all zero key. To run this
benchmark we execute: 

For the Prover:
---------------
cd empiricalZK/rtz14/linux/ ;
src/rtz14 -circuit ../test/AES -witness 00000000000000000000000000000000 -port 2020

For the verifier:
-----------------
cd empiricalZK/rtz14/linux/ ;
src/rtz14 -circuit ../test/AES -port 2020 -ip <address of the prover> 

(e.g. 127.0.0.1 if run on the same machine)


