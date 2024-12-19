#!/bin/bash

RES="experiment_results.txt"

N_THREADS=1
QPS=500

if [ -f $RES ]; then
    rm $RES
fi

for ((clients = 1; clients < 21; clients++)); do
    echo EXP -- CLIENTS: ${clients} N_THREADS: ${N_THREADS} QPS: ${QPS} 
    sudo ./run_networked.sh $clients $N_THREADS $QPS > sal 2>&1
    
    cat sal | grep "RESULT"
    cat sal | grep "RESULT" >> ${RES}
    sleep 5
done

echo "----------------------------" >> ${RES}

N_THREADS=2
QPS=1000

for ((clients = 1; clients < 11; clients++)); do
    echo EXP -- CLIENTS: ${clients} N_THREADS: ${N_THREADS} QPS: ${QPS}
    
    sudo ./run_networked.sh $clients $N_THREADS $QPS > sal 2>&1

    cat sal | grep "RESULT"
    cat sal | grep "RESULT" >> ${RES}
    sleep 5
done
