#!/bin/bash
# ops-per-worker is set to a very large value, so that TBENCH_MAXREQS controls how
# many ops are performed
NUM_WAREHOUSES=1

SERVER_PORT=${1}
WUP_REQ=${2}
MAX_REQ=${3}
THREADS=${4}

TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_MAXREQS=${MAX_REQ} TBENCH_WARMUPREQS=${WUP_REQ} ./out-perf.masstree/benchmarks/dbtest_server_networked --verbose --bench tpcc --num-threads ${THREADS} --scale-factor ${NUM_WAREHOUSES} --retry-aborted-transactions --ops-per-worker 10000000 &

sleep 2 # Allow server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"

