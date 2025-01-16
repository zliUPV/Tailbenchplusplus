#!/bin/bash

source /home/master/Documents/Tailbenchplusplus/configs.sh

SERVER_PORT=${1}
THREADS=${2}

REQS=100000000 # Set this very high; the harness controls maxreqs

TBENCH_SERVER_PORT=${SERVER_PORT} ./img-dnn_server_networked -r ${THREADS} \
  -f /home/master/Documents/tailbench.inputs/img-dnn/models/model.xml -n ${REQS} &

sleep 2

echo "[SERVER] STARTED. Clients can start now..."
wait $!
echo "[SERVER] FINISHED"
