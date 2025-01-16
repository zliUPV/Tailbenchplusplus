#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${DIR}/../configs.sh

PORT=${1}
THREADS=${2}
AUDIO_SAMPLES='audio_samples'

LD_LIBRARY_PATH=./sphinx-install/lib:${LD_LIBRARY_PATH} \
  TBENCH_SERVER_PORT=${PORT} ./decoder_server_networked -t ${THREADS} &

sleep 5
echo "[SERVER] : STARTED. Clients can start now..."
wait $!
echo "[SERVER] : FINISHED"
