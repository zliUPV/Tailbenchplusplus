#!/bin/bash

export LD_LIBRARY_PATH=/vmssd/tailbench_server/tailbench-v0.9/xapian/xapian-core-1.2.13/install/lib

source /vmssd/tailbench_server/tailbench-v0.9/configs.sh

SERVER_PORT=${1}
WUP_REQ=${2}
MAX_REQ=${3}
THREADS=${4}

TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_MAXREQS=${MAX_REQ} TBENCH_WARMUPREQS=${WUP_REQ} /vmssd/tailbench_server/tailbench-v0.9/xapian/xapian_networked_server -n ${THREADS} -d ${DATA_ROOT}/xapian/wiki -r 1000000000 &

sleep 2 # Wait for server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"


