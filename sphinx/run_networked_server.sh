#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/../configs.sh

PORT=${1}
THREADS=${2}

AUDIO_SAMPLES='audio_samples'

LD_LIBRARY_PATH=./sphinx-install/lib:${LD_LIBRARY_PATH} \
    ./decoder_server_networked -p ${PORT} -t ${THREADS} &