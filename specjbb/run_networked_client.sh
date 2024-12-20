#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/../configs.sh

# Setup commands
mkdir -p results

# Run specjbb
TBENCH_PATH=../harness

export LD_LIBRARY_PATH=${TBENCH_PATH}:${LD_LIBRARY_PATH}

export CLASSPATH=./build/dist/jbb.jar:./build/dist/check.jar:${TBENCH_PATH}/tbench.jar

export PATH=${JDK_PATH}/bin:${PATH}

SERVER=${1}
SERVER_PORT=${2}
QPS=${3}
THREADS=${4}

export TBENCH_QPS=${QPS}
export TBENCH_SERVER=${SERVER}
export TBENCH_SERVER_PORT=${SERVER_PORT}
export TBENCH_CLIENT_THREADS=${THREADS}
export TBENCH_MINSLEEPNS=10000



if [[ -d libtbench_jni.so ]] 
then
    rm libtbench_jni.so
fi
ln -sf libtbench_networked_jni.so libtbench_jni.so

./client &

echo "[CLIENT] : STARTED"
wait $!
echo "[CLIENT] : FINISHED"


rm libtbench_jni.so
rm gc.log
rm -r results

