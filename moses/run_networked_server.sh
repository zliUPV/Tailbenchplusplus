#!/bin/bash

SERVER_PORT=${1}
THREADS=${2}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/../configs.sh

BINDIR=./bin

# Setup
cp moses.ini.template moses.ini
sed -i -e "s#@DATA_ROOT#$DATA_ROOT#g" moses.ini

# Launch Server
TBENCH_SERVER_PORT=${PORT} chrt -r 99 ${BINDIR}/moses_server_networked -config ./moses.ini \
    -input-file ${DATA_ROOT}/moses/testTerms \
    -threads ${THREADS} -num-tasks 1000000 -verbose 0 &