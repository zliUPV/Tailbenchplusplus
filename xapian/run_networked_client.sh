#!/bin/bash

source /vmssd/tailbench_client/tailbench-v0.9/configs.sh


SERVER=${1}
SERVER_PORT=${2}
QPS=${3}
THREADS=${4}


TBENCH_VARQPS=0 TBENCH_INIQPS=0 TBENCH_INTERVALQPS=0 TBENCH_STEPQPS=0 TBENCH_SERVER=${SERVER} TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_QPS=${QPS} TBENCH_SERVER_THREADS=${THREADS} TBENCH_MINSLEEPNS=100000 TBENCH_TERMS_FILE=${DATA_ROOT}/xapian/terms.in /vmssd/tailbench_client/tailbench-v0.9/xapian/xapian_networked_client  &


echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"
