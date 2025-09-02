#!/bin/bash
# ops-per-worker is set to a very large value, so that TBENCH_MAXREQS controls how
# many ops are performed
NUM_WAREHOUSES=1

SERVER_PORT=${1}
THREADS=${2}

TBENCH_SERVER_PORT=${SERVER_PORT} ./out-perf.masstree/benchmarks/dbtest_server_networked --verbose --bench tpcc --num-threads ${THREADS} --scale-factor ${NUM_WAREHOUSES} --retry-aborted-transactions --ops-per-worker 10000000 &

sleep 2 # Allow server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"
