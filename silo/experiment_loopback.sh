#!/bin/bash


CLIENTS=$1
N_THREADS=1

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
sal=$(mktemp)


for ((QPS_total=500; QPS_total<20001; QPS_total=QPS_total)); do
    QPS=$(echo "scale=0; ${QPS_total}/${CLIENTS}" | bc -l)
    
    echo EXP SILO -- CLIENTS: ${CLIENTS} CLIENT_THREADS: ${N_THREADS} OVERALL_QPS: ${QPS_total} QPS_X_CLIENT: ${QPS}
    
    sudo ${DIR}/run_networked_loopback.sh $CLIENTS $N_THREADS $QPS > ${sal} 2>&1
    
    cat ${sal} | grep "RESULT"

    if [ $QPS_total -lt 10000 ]; then
	    QPS_total=$(echo "$QPS_total+500" | bc -l)
    else
	    QPS_total=$(echo "$QPS_total+1000" | bc -l)
    fi

    sleep 2
done

