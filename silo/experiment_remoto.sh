#!/bin/bash

RES="experiment_remoto_results_br0.txt"

N_THREADS=1
CLIENTS=7


if [ -f $RES ]; then
    rm $RES
fi


# 7 clients, QPS per client variable

for ((N_THREADS=1; N_THREADS<3; N_THREADS++)); do

    echo "----------------------------" >> ${RES}
    echo "Number of threads per client: ${N_THREADS}"
    echo "----------------------------" >> ${RES}

    for ((CLIENTS=7; CLIENTS<28; CLIENTS=CLIENTS+7)); do

        echo "----------------------------" >> ${RES}
        echo "Number of clients: ${CLIENTS}"
        echo "----------------------------" >> ${RES}
    
        for ((QPS_total=500; QPS_total<15001; QPS_total=QPS_total+500)); do
            
            QPS=$(echo "scale=0; ${QPS_total}/${CLIENTS}" | bc -l)
            echo EXP -- CLIENTS: ${clients} N_THREADS: ${N_THREADS} QPS: ${QPS}
        
            sudo ./run_networked_server.sh $clients $N_THREADS $QPS > sal 2>&1
        
            cat sal | grep "RESULT"
            cat sal | grep "RESULT" >> ${RES}
            sleep 5
        done
    done
done

