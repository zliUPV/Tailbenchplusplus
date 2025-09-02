#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${DIR}/../configs.sh

PORT=${1}
THREADS=${2}

DUMMYREQS=1000000

rm -f log scratch cmdfile db-tpcc-1 diskrw shore.conf info server.pid client.pid

TMP=$(mktemp -d --tmpdir=${SCRATCH_DIR})
ln -s $TMP scratch

mkdir scratch/log && ln -s scratch/log log
mkdir scratch/diskrw && ln -s scratch/diskrw diskrw

cp ${DATA_ROOT}/shore/db-tpcc-1 scratch/ &&
  ln -s scratch/db-tpcc-1 db-tpcc-1
chmod 644 scratch/db-tpcc-1

cp shore-kits/run-templates/cmdfile.template cmdfile
sed -i -e "s#@NTHREADS#$THREADS#g" cmdfile
sed -i -e "s#@REQS#$DUMMYREQS#g" cmdfile

cp shore-kits/run-templates/shore.conf.template shore.conf
sed -i -e "s#@NTHREADS#$THREADS#g" shore.conf

chrt -r 99 ./shore-kits/shore_kits_server_networked -i cmdfile &
sleep 5
echo "[SERVER]: Clients can start...."
wait $!
echo "[SERVER]: Server finished"
