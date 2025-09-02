#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/../configs.sh

AUDIO_SAMPLES='audio_samples'

SERVER=${1}
PORT=${2}
WARMUP=${3}
MAXREQ=${4}
QPS=${5}
THREADS=${6}

TBENCH_QPS=${QPS} TBENCH_SERVER=${SERVER} TBENCH_SERVER_PORT=${PORT} TBENCH_AN4_CORPUS=${DATA_ROOT}/sphinx \
    TBENCH_AUDIO_SAMPLES=${AUDIO_SAMPLES} TBENCH_WARMUPREQS=${WARMUP} \
    TBENCH_MAXREQS=${MAXREQ} TBENCH_THREADS=${THREADS} \
    ./decoder_client_networked &

