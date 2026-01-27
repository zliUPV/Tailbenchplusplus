#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${DIR}/../configs.sh

SERVER_PORT=${1}
THREADS=${2}

REQS=100000000 # Set this very high; the harness controls maxreqs

TBENCH_SERVER_PORT=${SERVER_PORT} ./img-dnn_server_networked -r ${THREADS} \
  -f ${DATA_ROOT}/img-dnn/models/model.xml -n ${REQS} &

sleep 2

echo "[SERVER] STARTED. Clients can start now..."
wait $!
echo "[SERVER] FINISHED"
