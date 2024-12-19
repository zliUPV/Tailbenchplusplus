#!/bin/bash
# ops-per-worker is set to a very large value, so that TBENCH_MAXREQS controls how
# many ops are performed

if [ "$#" -ne 3 ]; then
    echo "Usage: ./run_networked.sh N_CLIENTS TBENCH_CLIENT_THREADS CLIENT_QPS."
    exit 0
fi

NUM_WAREHOUSES=1
NUM_THREADS=1

N_CLIENTS=$1
TBENCH_CLIENT_THREADS=$2
QPS=$3

WARMUPREQS=$((N_CLIENTS*QPS*4))
MAXREQS=$((WARMUPREQS*3))

threads=$(echo "${N_CLIENTS}*${TBENCH_CLIENT_THREADS=}" | bc -l)

if [ $threads -lt 6 ]; then
    cores_taskset=7-11,31-35
else
    cores_taskset=7-11,31-35,17-23,41-47
fi

echo RESULT N_CLIENTS: ${N_CLIENTS} QPS: ${QPS} WARMUPREQS: ${WARMUPREQS} MAXREQS: ${MAXREQS} CORES: ${cores_taskset}


TBENCH_MAXREQS=${MAXREQS} TBENCH_WARMUPREQS=${WARMUPREQS} TBENCH_NCLIENTS=${N_CLIENTS} \
    taskset -c 6,30 ./out-perf.masstree/benchmarks/dbtest_server_networked --verbose --bench \
    tpcc --num-threads ${NUM_THREADS} --scale-factor ${NUM_WAREHOUSES} \
    --retry-aborted-transactions --ops-per-worker 10000000 2>&1 | tee server.sal &

echo $! > server.pid

sleep 5 # Allow server to come up

for ((i=0; i<${N_CLIENTS}; i++)); do
	TBENCH_QPS=${QPS} TBENCH_MINSLEEPNS=100 TBENCH_CLIENT_THREADS=${TBENCH_CLIENT_THREADS} TBENCH_ID=${i} taskset -c ${cores_taskset} ./out-perf.masstree/benchmarks/dbtest_client_networked 2>&1 | tee client_${i}.sal &
	echo $! > client_${i}.pid
done

wait $(cat server.pid)

# Clean up
for ((i=0; i<${N_CLIENTS}; i++)); do
	kill -9 $(cat client_${i}.pid) > /dev/null 2>&1
	rm client_${i}.pid
done

kill -9 $(cat server.pid) > /dev/null 2>&1
rm server.pid

worst_lat=0
worst_client=-1
avg_lat=0
avg_queue=0
avg_service=0

for ((i=0; i<${N_CLIENTS}; i++)); do
	lat=$(../utilities/parselats.py lats_${i}.bin | awk '{print $4}')
	echo RESULT Client ${i} -- Latency ${lat} ms

    queue=$(cat lats.txt | awk 'BEGIN {s=0; n=0} {s=s+$1; n=n+1} END {print s/n}')
    service=$(cat lats.txt | awk 'BEGIN {s=0; n=0} {s=s+$3; n=n+1} END {print s/n}')

    
    if (( $(echo "$lat > $worst_lat" | bc -l) )); then
        worst_client=${i}
        worst_lat=${lat}
        worst_queue=${queue}
        worst_service=${service}
    fi

    avg_lat=$(echo "${avg_lat} + ${lat}" | bc -l)
    avg_queue=$(echo "${avg_queue} + ${queue}" | bc -l)
    avg_service=$(echo "${avg_service} + ${service}" | bc -l)
done

echo RESULT WORST Client ${worst_client} Tail_latency: ${worst_lat} Queue_time: ${worst_queue} Service_time: ${worst_service}

avg_lat=$(echo "${avg_lat} / ${N_CLIENTS}" | bc -l)
avg_queue=$(echo "${avg_queue} / ${N_CLIENTS}" | bc -l)
avg_service=$(echo "${avg_service} / ${N_CLIENTS}" | bc -l)

echo RESULT AVG Clients Tail_latency: ${avg_lat} Queue_time: ${avg_queue} Service_time: ${avg_service}

