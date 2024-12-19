#!/bin/bash

id=0

tmpfile=$(mktemp)

while [ -f "lats_${id}.bin" ]; do

    ~/tailbench/utilities/parselats.py "lats_${id}.bin" &>/dev/null
    cat lats.txt | tail -n +2 | awk '{print $3}' >> $tmpfile
    
    id=$(echo "$id+1" | bc -l)
done

lat=$(sort $tmpfile -n | awk 'BEGIN{c=0} length($0){a[c]=$0;c++}END{p5=(c/100*5); p5=p5%1?int(p5)+1:p5; print a[c-p5-1]}')
echo "AGG_95th_LAT" $lat
