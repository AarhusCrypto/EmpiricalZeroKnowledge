#!/bin/bash
#
# Development script.
#
# Linux/Unix are an IDE ! The shell is all the power you'll ever need.
#
# With these functions you are even more powerful 
#


reload() {
 source "$(realpath ${BASH_SOURCE})"
}

help(){
 echo "dbg - build current project in debug mode for GDB"
}

dbg() {
 if [ -d ./linux ]; then
     pushd linux
 fi
 make -f linux.mak clean debug BUILDDIR=${PWD}/../../../sys
 make install
 if [ -d ./linux ]; then
     popd
 fi
}

 alias run_prover="src/prover  -circuit ../test/AES -port 2021 -witness 000000000000000000000000000000000000000000000000000000000000000001"

alias run_verifier="src/prover  -circuit ../test/AES -port 2021 -ip 127.0.0.1"


