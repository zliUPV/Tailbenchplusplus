#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SERVER=${1}
SERVER_PORT=${2}
WARMUP=${3}
MAXREQ=${4}
QPS=${5}
THREADS=${6}

DUMMYREQS=1000000

rm -f log scratch cmdfile df-tpcc-1 diskrw shore.conf info client.pid
TMP=$(mktemp -d --tmpdir=${SCRATCH_DIR})
ln -s $TMP scratch

mkdir scratch/log && ln -s scratch/log log
mkdir scratch/diskrw && ln -s scratch/diskrw diskrw

cp ${DATA_ROOT}/shore/db-tpcc-1 scratch/ && ln -s scratch/db-tpcc-1 db-tpcc-1
chmod 644 scratch/db-tpcc-1

cp shore-kits/run-templates/cmdfile.template cmdfile
sed -i -e "s#@NTHREADS#$THREADS#g" cmdfile
sed -i -e "s#@REQS#$DUMMYREQS#g" cmdfile

cp shore-kits/run-templates/shore.conf.template shore.conf
sed -i -e "s#@NTHREADS#$THREADS#g" shore.conf

TBENCH_WARMUPREQS=${WARMUP} TBENCH_MAXREQS=${MAXREQ} TBENCH_SERVER=${SERVER} \
  TBENCH_SERVER_PORT=${SERVER_PORT} TBENCH_CLIENT_THREADS=${THREADS} TBENCH_QPS=${QPS} \
  TBENCH_MINSLEEPNS=10000 chrt -r 99 ./shore-kits/shore_kits_client_networked -i cmdfile &

echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"
