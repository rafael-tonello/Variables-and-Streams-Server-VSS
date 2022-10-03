#!/bin/bash

rundir=/root/bin
workdir=/root/homeaut2_workdir
telegramSender=/root/sendToTelegram.sh
logsFolder
clearObjectsBeforeMake=true

main()
{
    while [ true ]; do
        waitNextChange
        sendTelegram "Hey! A new VSS deploy pipeline was started!"
        tests
        if [ "$?" == "0" ]; then
            build
            if [ "$?" == "0" ]; then
                updateBinaries
            fi
        fi
    done
}

sendTelegram()
{
    echo $1
    $telegramSender "$1"
}

sendTelegramFile()
{
    echo "sending file '$1' to Telegram"
    $telegramSender sendFile "$1"
}

waitNextChange()
{
    while [ true ]; do
        cd $workdir
        local c1=$(git log -n 1 main --pretty=format:"%H")
        git pull
        local c2=$(git log -n 1 main --pretty=format:"%H")

        if [ "$c1" != "$c2" ]; then
            return
        fi

        sleep 10
    done
}

tests()
{
	echo "entering the folder $workdir/tests"
	cd "$workdir/tests"
	if [ "$clearObjectsBeforeMake" == "true" ]
	then
	    rm -rf ./objects
	fi
	   
    rm -rf ./build
	
	make all > /tmp/mkTestsResult.log 2>&1
	if [ "$?" == "0" ]
	then
		cd build
		./tests > /tmp/testsResult.log 2>&1
		if [ "$?" == "0" ]
		then
		    return 0
		else
    		sendTelegram ":(.. Tests excutions resulted in falure"
    		sendTelegramFile "/tmp/testsResult.log"
    		return 2
		fi
	else
		sendTelegram "Hmmm! It didn't even compile the tests"
		sendTelegramFile "/tmp/mkTestsResult.log"
		return 1
    fi
	
}

build()
{
	echo "build was called"
}

updateBinaries()
{
	echo "updateBinaries was called"
}

main
