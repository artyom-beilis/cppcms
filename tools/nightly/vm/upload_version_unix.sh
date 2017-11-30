#!/bin/bash -x

ssh -p $VM_PORT localhost rm -fr /tmp/nightly || exit 1
ssh -p $VM_PORT localhost mkdir /tmp/nightly || exit 1
scp -P $VM_PORT cppcms.tar.gz localhost:/tmp/nightly || exit 1

ssh -p $VM_PORT localhost $VM_TAR -xvf /tmp/nightly/cppcms.tar.gz -C /tmp/nightly  || exit 1
