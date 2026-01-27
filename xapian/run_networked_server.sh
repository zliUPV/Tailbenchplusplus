#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${DIR}/../configs.sh

export LD_LIBRARY_PATH=${DIR}/xapian-core-1.2.13/install/lib

SERVER_PORT=${1}
THREADS=${2}

TBENCH_SERVER_PORT=${SERVER_PORT} ./xapian_networked_server -n ${THREADS} -d ${DATA_ROOT}/xapian/wiki -r 1000000000 &

sleep 2 # Wait for server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"
