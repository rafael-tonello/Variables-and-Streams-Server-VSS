#!/bin/bash

erroCount=0
successCount=0
server="http://localhost:5024"

while true; do

    #random 1 to 10
    rnd=$((RANDOM % 10 + 1))
    if [ $rnd -le 8 ]; then
        curl -sS --max-time 5 "$server/*"  >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            ((successCount++))
        else
            ((erroCount++))
        fi
    else
        curl -sS --max-time 5 -X POST "$server/n0/serverTest/$RANDOM" -d $RANDOM >/dev/null 2>&1
    fi


    total=$((successCount + erroCount))
    percentSucess=$(echo "scale=2; ($successCount / $total) * 100" | bc)

    echo "Success: $successCount, Errors: $erroCount, Total: $total, Success Rate: $percentSucess%"


    sleep 0.15
done
