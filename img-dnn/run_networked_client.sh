#!/bin/bash

source /home/master/Documents/Tailbenchplusplus/configs.sh

SERVER=${1}
SERVER_PORT=${2}
WARMUP=${3}
MAXREQ=${4}
QPS=${5}
THREADS=${6}
TBENCH_ID=${7}

TBENCH_WARMUPREQS=${WARMUP} TBENCH_MAXREQS=${MAXREQ} TBENCH_VARQPS=0 \
  TBENCH_SERVER=${SERVER} TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_QPS=${QPS} TBENCH_CLIENT_THREADS=${THREADS} \
  TBENCH_MNIST_DIR=/home/master/Documents/tailbench.inputs/img-dnn/mnist TBENCH_ID=${TBENCH_ID} ./img-dnn_client_networked &

echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"
