#!/bin/bash
# ops-per-worker is set to a very large value, so that TBENCH_MAXREQS controls how
# many ops are performed

SERVER=${1}
SERVER_PORT=${2}
WARMUP=${3}
MAXREQ=${4}
QPS=${5}
THREADS=${6}

TBENCH_WARMUPREQS=${WARMUP} TBENCH_MAXREQS=${MAXREQ} TBENCH_SERVER=${SERVER} TBENCH_SERVER_PORT=${SERVER_PORT} \
  TBENCH_QPS=${QPS} TBENCH_CLIENT_THREADS=${THREADS} TBENCH_MINSLEEPNS=10000 ./out-perf.masstree/benchmarks/dbtest_client_networked &

echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"
