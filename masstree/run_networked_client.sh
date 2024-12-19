#!/bin/bash

SERVER=${1}
SERVER_PORT=${2}
QPS=${3}
THREADS=${4}


TBENCH_CLIENT_THREADS=${THREADS} TBENCH_SERVER=${SERVER} TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_QPS=${QPS} TBENCH_MINSLEEPNS=100000 ./mttest_client_networked &

echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"