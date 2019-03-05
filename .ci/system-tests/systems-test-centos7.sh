#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export CLIENT_OS=centos7
export CLIENT_HOSTNAME=hirs-client-$CLIENT_OS-tpm1.2

export SERVER_OS=$CLIENT_OS
export SERVER_HOSTNAME=hirs-appraiser-$SERVER_OS

export ENABLED_COLLECTORS=
export TPM_VERSION=1.2

$SCRIPT_DIR/systems-test.core.sh
