#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: ./run_networked.sh N_CLIENTS TBENCH_CLIENT_THREADS CLIENT_QPS."
    exit 0
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/../configs.sh

cd ${DIR}

# Setup commands
mkdir -p results

# Run specjbb
TBENCH_PATH=../harness


N_CLIENTS=${1}
TBENCH_CLIENT_THREADS=${2}
QPS=${3}
WARMUPREQS=$((N_CLIENTS*QPS*4))
MAXREQS=$((WARMUPREQS*3))


echo RESULT N_CLIENTS: ${N_CLIENTS} QPS: ${QPS} WARMUPREQS: ${WARMUPREQS} MAXREQS: ${MAXREQS}


export LD_LIBRARY_PATH=${TBENCH_PATH}:${LD_LIBRARY_PATH}

export CLASSPATH=./build/dist/jbb.jar:./build/dist/check.jar:${TBENCH_PATH}/tbench.jar

export PATH=${JDK_PATH}/bin:${PATH}

export TBENCH_QPS=${QPS}
export TBENCH_MAXREQS=${MAXREQS}              #25000 
export TBENCH_WARMUPREQS=${WARMUPREQS}        #25000 
export TBENCH_MINSLEEPNS=100

export TBENCH_NCLIENTS=${N_CLIENTS}
export TBENCH_CLIENT_THREADS=${TBENCH_CLIENT_THREADS}

if [[ -d libtbench_jni.so ]] 
then
    rm libtbench_jni.so
fi
ln -sf libtbench_networked_jni.so libtbench_jni.so


# LAUNCH THE SERVER
taskset -c 5,29 ${JDK_PATH}/bin/java -Djava.library.path=. -XX:ParallelGCThreads=1 -XX:+UseSerialGC -XX:NewRatio=1 -XX:NewSize=7000m -Xloggc:gc.log -Xms10000m -Xmx10000m -Xrs spec.jbb.JBBmain -propfile SPECjbb_mt.props &

echo $! > server.pid

sleep 10


# XPL4 has 2 (sockets) x 12 SMT cores (the first five of each processor reserved for the OS and DPDK)
# The server executes in cores 5 and 29
c1=6
c2=30
cpu_set=${c1},${c2}

for ((i=0; i<${N_CLIENTS}; i++)); do
    export TBENCH_ID=${i}

    # LAUNCH THE CLIENT
    taskset -c ${cpu_set} ${DIR}/client 2>&1 | tee client_${i}.sal &
    echo $! > client_${i}.pid

    # Update the cpu_set
    c1=$(echo "$c1+1" | bc -l)
    c2=$(echo "$c2+1" | bc -l)
    if [ $c1 -eq 12 ]; then
        c1=17
        c2=41
    elif [ $c1 -eq 24 ]; then
        c1=6
        c2=30
    fi
    cpu_set=${c1},${c2}
done


wait $(cat server.pid)


# Clean up
for ((i=0; i<${N_CLIENTS}; i++)); do
    kill -9 $(cat client_${i}.pid) > /dev/null 2>&1
    rm client_${i}.pid
done
kill -9 $(cat server.pid) > /dev/null 2>&1
rm server.pid

# Teardown
rm libtbench_jni.so
rm gc.log
rm -r results


# Process results
worst_lat=0
worst_client=-1
avg_lat=0
avg_queue=0
avg_service=0
avg_timeliness=0
worst_timeliness=2

for ((i=0; i<${N_CLIENTS}; i++)); do
    lat=$(../utilities/parselats.py lats_${i}.bin | awk '{print $4}')

    queue=$(cat lats.txt | awk 'BEGIN {s=0; n=0} {s=s+$1; n=n+1} END {print s/n}')
    service=$(cat lats.txt | awk 'BEGIN {s=0; n=0} {s=s+$3; n=n+1} END {print s/n}')

    timeliness=$(cat client_${i}.sal | grep "Percentatge timely requests" | awk '{print $11}')

    echo "RESULT Client ${i} -- Latency(ms): ${lat} Queue_time(ms): ${queue} Service_time(ms): ${service} Timeliness: ${timeliness}"
    
    if (( $(echo "$lat > $worst_lat" | bc -l) )); then
        worst_client=${i}
        worst_lat=${lat}
        worst_queue=${queue}
        worst_service=${service}
    fi

    if (( $(echo "$timeliness < $worst_timeliness" | bc -l) )); then
        worst_timeliness=$timeliness
    fi

    avg_lat=$(echo "${avg_lat} + ${lat}" | bc -l)
    avg_queue=$(echo "${avg_queue} + ${queue}" | bc -l)
    avg_service=$(echo "${avg_service} + ${service}" | bc -l)
    avg_timeliness=$(echo "${avg_timeliness} + ${timeliness}" | bc -l)
done

echo RESULT WORST Client ${worst_client} Tail_latency: ${worst_lat} Queue_time: ${worst_queue} Service_time: ${worst_service} Timeliness: ${worst_timeliness}

avg_lat=$(echo "${avg_lat} / ${N_CLIENTS}" | bc -l)
avg_queue=$(echo "${avg_queue} / ${N_CLIENTS}" | bc -l)
avg_service=$(echo "${avg_service} / ${N_CLIENTS}" | bc -l)
avg_timeliness=$(echo "${avg_timeliness} / ${N_CLIENTS}" | bc -l)

echo RESULT AVG Clients Tail_latency: ${avg_lat} Queue_time: ${avg_queue} Service_time: ${avg_service} Timeliness: ${avg_timeliness}


# Remove the client and lats files
sudo rm -r ./lats_*
sudo rm	lats.txt
sudo rm	-r ./client_*
