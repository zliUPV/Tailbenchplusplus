#!/bin/bash

SERVER=${1}
SERVER_PORT=${2}
WARMUP=${3}
MAXREQS=${4}
QPS=${5}
THREADS=${6}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/../configs.sh

BINDIR=./bin

# Setup
cp moses.ini.template moses.ini
sed -i -e "s#@DATA_ROOT#$DATA_ROOT#g" moses.ini

TBENCH_QPS=${QPS} TBENCH_MINSLEEPNS=10000 TBENCH_WARMUPREQS=${WARMUPREQS} \
    TBENCH_MAXREQS=${MAXREQS} chrt -r 99 ${BINDIR}/moses_client_networked &
    
