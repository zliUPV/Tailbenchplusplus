#!/bin/bash

DIR=/home/master/Documents/Tailbenchplusplus/
source ${DIR}configs.sh

SERVER=${1}
SERVER_PORT=${2}
WARMUP=${3}
MAXREQ=${4}
QPS=${5}
THREADS=${6}

AUDIO_SAMPLES='/home/master/Documents/Tailbenchplusplus/sphinx/audio_samples'

TBENCH_WARMUPREQS=${WARMUP} TBENCH_MAXREQS=${MAXREQ} TBENCH_ID=0 TBENCH_SERVER=${SERVER} \
  TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_CLIENT_THREADS=${THREADS} \
  TBENCH_QPS=${QPS} TBENCH_MINSLEEPNS=10000 TBENCH_AN4_CORPUS=${DATA_ROOT}/sphinx \
  TBENCH_AUDIO_SAMPLES=${AUDIO_SAMPLES} ./decoder_client_networked &
echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"
