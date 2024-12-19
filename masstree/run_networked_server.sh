#!/bin/bash

SERVER_PORT=${1}
WUP_REQ=${2}
MAX_REQ=${3}
THREADS=${4}

TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_MAXREQS=${MAX_REQ} TBENCH_WARMUPREQS=${WUP_REQ} ./mttest_server_networked -j${THREADS} mycsba masstree &

sleep 5 # Allow server to come up

echo "[SERVER] : STARTED. Clients can start now..."

wait $!

echo "[SERVER] : FINISHED"
