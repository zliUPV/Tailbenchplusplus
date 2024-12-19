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

SERVER_PORT=${1}
WUP_REQ=${2}
MAX_REQ=${3}
THREADS=${4}

export TBENCH_SERVER_PORT=${SERVER_PORT}
export TBENCH_MAXREQS=${MAX_REQ} 
export TBENCH_WARMUPREQS=${WUP_REQ}
export TBENCH_MINSLEEPNS=10000

if [[ -d libtbench_jni.so ]] 
then
    rm libtbench_jni.so
fi
ln -sf libtbench_networked_jni.so libtbench_jni.so

${JDK_PATH}/bin/java -Djava.library.path=. -XX:ParallelGCThreads=${THREADS} -XX:+UseSerialGC -XX:NewRatio=1 -XX:NewSize=7000m -Xloggc:gc.log -Xms10000m -Xmx10000m -Xrs spec.jbb.JBBmain -propfile SPECjbb_mt.props &

sleep 2 # Wait for server to come up
echo "[SERVER] : STARTED. Clients can start now."
wait $!
echo "[SERVER] : FINISHED"


rm libtbench_jni.so
rm gc.log
rm -r results

