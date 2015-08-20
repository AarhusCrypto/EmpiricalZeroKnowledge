#!/bin/bash
#
# Development script.
#
# Source this file in your bash environment to has easy shortcuts.
#
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


