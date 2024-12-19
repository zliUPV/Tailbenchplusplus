#!/bin/bash


CLIENTS=$1
N_THREADS=1


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
sal=$(mktemp)


for ((QPS_total=100; QPS_total<1501; QPS_total=QPS_total+100)); do

    QPS=$(echo "scale=0; ${QPS_total}/${CLIENTS}" | bc -l)

    echo EXP IMG-DNN -- CLIENTS: ${CLIENTS} CLIENT_THREADS: ${N_THREADS} OVERALL_QPS: ${QPS_total} QPS_X_CLIENT: ${QPS}
        
    sudo ${DIR}/run_networked_network_server.sh $CLIENTS $N_THREADS $QPS > ${sal} 2>&1
    
    cat ${sal} | grep "AGG_95th_LAT"
        
    sleep 2
done

