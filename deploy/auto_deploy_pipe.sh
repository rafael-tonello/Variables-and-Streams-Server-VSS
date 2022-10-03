#!/bin/bash

rundir=/home/vss/bin
workdir=/tmp/vss/work
telegramSender=~/scripts/sendToTelegram.sh

main()
{
    while [ true ]; do
        waitNextChange
        sendToTelegram "Hey! A new VSS deploy pipeline was started!"
        tests
        if [ "$?" == "0"]; then
            build
            if [ "$?" == "0"]; then
                updateBinaries
            fi
        fi
    done
}

sendTelegram()
{
    eval "$telegramSender $1"
}

waitNextChange()
{
    while [ true ]; do
        cd $workdir
        local c1=$(git log -n 1 main --pretty=format:"%H")
        git pull
        local c2=$(git log -n 1 main --pretty=format:"%H")

        if [ "$c1" == "$c2" ]; then
            return
        fi

        sleep 10
    done
}

tests()
{

}

build()
{

}

updateBinaries()
{

}

main
