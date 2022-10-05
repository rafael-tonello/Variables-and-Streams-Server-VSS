#!/bin/bash

rundir=/root/bin
binaryName=VarServer
workdir=/root/homeaut2_workdir
telegramSender=/root/sendToTelegram.sh
clearObjectsBeforeMake=true
versionCheckInteval_seconds=60

_return=""
nl=$'\n'

main()
{
    #check if $binaryName exists (first deploy)
    if [ ! -f "$rundir/$binaryName" ]; then
        sendTelegram "Binary file not found. Starting first deploy"
        deploy
    fi

    while [ true ]; do
        waitNextChange
        sendTelegram "Hey! A new VSS deploy pipeline was started!"
        deploy
    done
}

deploy()
{
    tests
    if [ "$?" == "0" ]; then
        echo "building app"
        build
        if [ "$?" == "0" ]; then
            updateBinariesAndRun
            if [ "$?" == "0" ]; then                    
                sendTelegram "🙏😄😄 Ok! The deployment was successful! 😄😄🙏"
                make_stage_failure_chart "sucess"
                sendTelegram "$_return"
            else
                sendTelegram "🆘🆘🆘🆘🆘😕😕😕😕🆘🆘🆘🆘🆘$nl👀Unfortunately the deploy process can't be completed👀"
            fi
        else
            sendTelegram "🆘🆘🆘🆘🆘😕😕😕😕🆘🆘🆘🆘🆘$nl👀Unfortunately the deploy process can't be completed👀"
        fi
    else
        sendTelegram "🆘🆘🆘🆘🆘😕😕😕😕🆘🆘🆘🆘🆘$nl👀Unfortunately the deploy process can't be completed👀"
    fi
}

make_stage_failure_chart()
{
    legend="  1) Build tests$nl  2) Run tests$nl  3) Build app$nl  4) Update binaries$nl  5) Run new binary version"
#    err=✕
    err=x
    sus=✓
#    err=⛔️
#    sus=✅
    if [ "$1" == "buildTests" ]; then
        _return="[$err]-->[2]-->[3]-->[4]-->[5]$nl$legend"
    elif [ "$1" == "runTests" ]; then
        _return="[$sus]==>[$err]-->[3]-->[4]-->[5]$nl$legend"
    elif [ "$1" == "buildApp" ]; then
        _return="[$sus]==>[$sus]==>[$err]-->[4]-->[5]$nl$legend"
    elif [ "$1" == "updateBinaries" ]; then
        _return="[$sus]==>[$sus]==>[$sus]==>[$err]-->[5]$nl$legend"
    elif [ "$1" == "run" ]; then
        _return="[$sus]==>[$sus]==>[$sus]==>[$sus]==>[$err]$nl$legend"
    else
        _return="[$sus]==>[$sus]==>[$sus]==>[$sus]==>[$sus]$nl$legend"
    fi
}

sendTelegram()
{
    echo "Sending to Telegram: $1"
    $telegramSender "$1" > /dev/null 2>&1
    sleep 5
}

sendTelegramFile()
{
    echo "Sending file '$1' to Telegram"
    $telegramSender sendFile "$1" > /dev/null 2>&1
    sleep 5
}

waitNextChange()
{
    while [ true ]; do
        

        cd $workdir
        local c1=$(git log -n 1 main --pretty=format:"%H")
        git pull
        local c2=$(git log -n 1 main --pretty=format:"%H")

        if [ "$c1" != "$c2" ]; then
            return 0
        fi

        sleep $versionCheckInteval_seconds
    done
}

tests()
{
	echo "Entering in the folder $workdir/tests"
	cd "$workdir/tests"
	if [ "$clearObjectsBeforeMake" == "true" ]
	then
	    rm -rf ./objects
	fi
	  
	echo "Building tests"
    rm -rf ./build
	
	make all > /tmp/mkTestsResult.log 2>&1
	if [ "$?" == "0" ]
	then
		cd build
		echo "Running tests"
		./tests > /tmp/testsResult.log 2>&1
		if [ "$?" == "0" ]
		then
		    return 0
		else
		    make_stage_failure_chart "runTests"
    	    sendTelegram "$_return"
    		sendTelegram "😕.. Tests excutions resulted in falure"
    		sendTelegramFile "/tmp/testsResult.log"
    		return 2
		fi
	else
	    make_stage_failure_chart "buildTests"
	    sendTelegram "$_return"
		sendTelegram "Hmmm! It didn't even compile the tests"
		sendTelegramFile "/tmp/mkTestsResult.log"
		return 1
    fi
	
}

build()
{
	echo "Entering in the folder $workdir"
	cd "$workdir"
	if [ "$clearObjectsBeforeMake" == "true" ]
	then
	    rm -rf ./objects
	fi
	   
	echo "Building main project"
    rm -rf ./build
	
	make all > /tmp/mkResult.log 2>&1
	if [ "$?" == "0" ]
	then
	    return 0
	else
	    make_stage_failure_chart "buildApp"
	    sendTelegram "$_return"
		sendTelegram "Tests works well, but we had erros compiling the main project"
		sendTelegramFile "/tmp/mkResult.log"
		return 1
    fi
}

updateBinariesAndRun()
{
    echo "Updating binaries"
    kill $(pgrep $binaryName)
    rm -rf $rundir.bak
    cp -r $rundir $rundir.bak
    
    cp -f "$workdir/build/"* "$rundir/"
    echo "Running main App"
    cd $rundir
    ./$binaryName &
    sleep 10
    pgrep $binaryName
    pgr=$?
    echo "pgr=$pgr"
    if [ "$pgr" == "0" ]; then
        return 0
    else
        revertBinaryBackup &
        rbbPid=$!
        sendTelegram "⚠ The new binary version ran with failure ⚠" &
        sleep 1
        make_stage_failure_chart "run"
	    sendTelegram "$_return" &
	    sleep 1
        sendTelegram "Rollbacking to old binary version"
        
        echo "waiting rollback"
        waitPid $rbbPid
        echo "rollback done"
        return 1
    fi
}

revertBinaryBackup()
{
    echo "Rollbacking"
    rm -rf $rundir.withProblem
    mv $rundir $rundir.withProblem
    mv $rundir.bak $rundir
    cd $rundir
    echo "Running restored main App"
    ./$binaryName &
    sleep 10;
    pgrep $binaryName
        
    if [ "$?" == "0" ]; then
    	sendTelegram "Rollback done with sucess"
        return 0
    else
        sendTelegram "⚠⚠⚠⚠⚠⚠⚠ The rollback failed ⚠⚠⚠⚠⚠⚠⚠"
#        sendTelegram "⚠⚠⚠⚠⚠⚠⚠ The rollback failed ⚠⚠⚠⚠⚠⚠⚠"
#        sendTelegram "⚠⚠⚠⚠⚠⚠⚠ The rollback failed ⚠⚠⚠⚠⚠⚠⚠"
#        sendTelegram "⚠⚠⚠⚠⚠⚠⚠ The rollback failed ⚠⚠⚠⚠⚠⚠⚠"
#        sendTelegram "⚠⚠⚠⚠⚠⚠⚠ The rollback failed ⚠⚠⚠⚠⚠⚠⚠"
        return 1
    fi
}

waitPid()
{
    while [ true ]; do
        ps $1 > /dev/null
        if [ "$?" != "0" ]; then
            return 0
        fi
        sleep 0.5
    done
}

main