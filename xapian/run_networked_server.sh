#!/bin/bash

export LD_LIBRARY_PATH=/vmssd/tailbench_server/tailbench-v0.9/xapian/xapian-core-1.2.13/install/lib

source /vmssd/tailbench_server/tailbench-v0.9/configs.sh

SERVER_PORT=${1}
THREADS=${4}

TBENCH_SERVER_PORT=${SERVER_PORT} /vmssd/tailbench_server/tailbench-v0.9/xapian/xapian_networked_server -n ${THREADS} -d ${DATA_ROOT}/xapian/wiki -r 1000000000 &

sleep 2 # Wait for server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"


