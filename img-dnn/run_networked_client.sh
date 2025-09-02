#!/bin/bash

source /vmssd/tailbench_server/tailbench-v0.9/configs.sh

SERVER=${1}
SERVER_PORT=${2}
WARMUP=${3}
MAXREQS=${4}
QPS=${5}
THREADS=${6}

TBENCH_SERVER=${SERVER} TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_WARMUPREQS=${WARMUP} \
TBENCH_MAXREQS=${MAXREQS} TBENCH_QPS=${QPS} TBENCH_CLIENT_THREADS=${THREADS} TBENCH_MNIST_DIR=/vmssd/tailbench_data/tailbench.inputs/img-dnn/mnist /vmssd/tailbench_server/tailbench-v0.9/img-dnn/img-dnn_client_networked &

echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"
