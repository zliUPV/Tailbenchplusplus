#!/bin/bash

#DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ../configs.sh

export LD_LIBRARY_PATH=xapian-core-1.2.13/install/lib

NSERVERS=1
QPS=500
WARMUPREQS=100
REQUESTS=2500

TBENCH_MAXREQS=${REQUESTS} TBENCH_WARMUPREQS=${WARMUPREQS} chrt -r 99 ./xapian_networked_server -n ${NSERVERS} -d ~/Documentos/Huawei_project/tailbench.inputs/xapian/wiki -r 1000000000 &
echo $! > server.pid

sleep 5 # Wait for server to come up

TBENCH_QPS=${QPS} TBENCH_MINSLEEPNS=100000 TBENCH_TERMS_FILE=~/Documentos/Huawei_project/tailbench.inputs/xapian/terms.in chrt -r 99 ./xapian_networked_client &

echo $! > client.pid

wait $(cat client.pid)

# Clean up
./kill_networked.sh
rm server.pid client.pid
