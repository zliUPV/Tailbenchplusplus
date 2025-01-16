#!/bin/bash

SERVER_PORT=${1}
THREADS=${2}

TBENCH_SERVER_PORT=${SERVER_PORT} ./mttest_server_networked -j${THREADS} mycsba masstree &

sleep 5 # Allow server to come up

echo "[SERVER] : STARTED. Clients can start now..."

wait $!

echo "[SERVER] : FINISHED"
