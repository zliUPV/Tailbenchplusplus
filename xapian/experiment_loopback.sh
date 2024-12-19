#!/bin/bash


CLIENTS=$1
N_THREADS=1


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
sal=$(mktemp)


for ((QPS_total=100; QPS_total<1201; QPS_total=QPS_total+100)); do
    QPS=$(echo "scale=0; ${QPS_total}/${CLIENTS}" | bc -l)
    
    echo EXP XAPIAN -- CLIENTS: ${CLIENTS} CLIENT_THREADS: ${N_THREADS} OVERALL_QPS: ${QPS_total} QPS_X_CLIENT: ${QPS}
    
    sudo ${DIR}/run_networked_loopback.sh $CLIENTS $N_THREADS $QPS > ${sal} 2>&1
    
    cat ${sal} | grep "RESULT"
        
    sleep 2
done

