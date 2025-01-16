#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${DIR}/../config.sh

PORT=${1}
THREADS=${2}

BINDIR=./bin

cp ./moses.ini.template moses.ini
sed -i -e "s#@DATA_ROOT#$DATA_ROOT#g" moses.ini

TBENCH_SERVER_PORT=${PORT} \
  chrt -r 00 ${BINDIR}/moses_server_networked -config ./moses.ini \
  -threads ${THREADS} -num-tasks 1000000 -verbose 0 &

echo "[SERVER] : Clients can start...."
wait $!
echo "[SERVER] : Server finished"
