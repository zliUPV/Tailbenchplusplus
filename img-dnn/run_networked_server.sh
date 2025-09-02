#!/bin/bash

source /vmssd/tailbench_server/tailbench-v0.9/configs.sh


SERVER_PORT=${1}
THREADS=${2}

REQS=100000000 # Set this very high; the harness controls maxreqs

TBENCH_SERVER_PORT=${SERVER_PORT} /vmssd/tailbench_server/tailbench-v0.9/img-dnn/img-dnn_server_networked -r ${THREADS} -f /vmssd/tailbench_data/tailbench.inputs/img-dnn/models/model.xml -n ${REQS} &

sleep 2

echo "[SERVER] STARTED. Clients can start now..."
wait $!
echo "[SERVER] FINISHED"

