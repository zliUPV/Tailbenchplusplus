#!/bin/bash

export LD_LIBRARY_PATH=/home/master/Documents/Tailbenchplusplus/xapian/xapian-core-1.2.13/install/lib

source /home/master/Documents/Tailbenchplusplus/configs.sh

SERVER_PORT=${1}
THREADS=${2}

TBENCH_SERVER_PORT=${SERVER_PORT} ./xapian_networked_server -n ${THREADS} -d ${DATA_ROOT}/xapian/wiki -r 1000000000 &

sleep 2 # Wait for server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"
