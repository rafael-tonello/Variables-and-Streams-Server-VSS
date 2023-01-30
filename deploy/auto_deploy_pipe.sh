#!/bin/bash

rundir=/home/orangepi/vss/run
binaryName=VarServer
mainLogFile=/home/orangepi/vss/run/vss.log
workdir=/home/orangepi/vss/workdir
telegramSender=/home/orangepi/scripts/sendToTelegram.sh
clearObjectsBeforeMake=true
versionCheckInteval_seconds=60

_return=""
nl=$'\n'
currAppState="running"
makeAndTestNiceness=19

main()
{
    #check if $binaryName exists (first deploy)
    if [ ! -f "$rundir/$binaryName" ]; then
        sendTelegram "Binary file not found. Starting first deploy"
        deploy
    fi

    checkAppRunning &

    while [ true ]; do
        waitNextChange
        sendTelegram "Hey! A new VSS deploy pipeline was started!"
        deploy
    done
}

deploy()
{
    currAppState="deploying"
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
                return 0
            else
                sendTelegram "🆘🆘🆘🆘🆘😕😕😕😕🆘🆘🆘🆘🆘$nl👀Unfortunately the deploy process can't be completed👀"
                return 1
            fi
        else
            sendTelegram "🆘🆘🆘🆘🆘😕😕😕😕🆘🆘🆘🆘🆘$nl👀Unfortunately the deploy process can't be completed👀"
            return 2
        fi
    else
        sendTelegram "🆘🆘🆘🆘🆘😕😕😕😕🆘🆘🆘🆘🆘$nl👀Unfortunately the deploy process can't be completed👀"
        return 3
    fi
    return 4
}

waitNextChange()
{
    while [ true ]; do

        cd $workdir
        local c1=$(git log -n 1 main --pretty=format:"%H")
        git pull
        local c2=$(git log -n 1 main --pretty=format:"%H")

        if [ "$c1" != "$c2" ]; then
            gitCompleteUpdate
            return 0
        fi

        sleep $versionCheckInteval_seconds
    done
}

gitCompleteUpdate()
{
    cd $workdir
    repoUrl=$(git config remote.origin.url)
    cd ..
    rm -rf $workdir
    git clone $repoUrl $workdir
    cd $workdir
    git submodule update --init
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
	
	nice -$makeAndTestNiceness make all > /tmp/mkTestsResult.log 2>&1
	if [ "$?" == "0" ]
	then
		cd build
		echo "Running tests"
		nice -$makeAndTestNiceness ./tests > /tmp/testsResult.log 2>&1
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
	
	nice -$makeAndTestNiceness make all > /tmp/mkResult.log 2>&1
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
    currAppState="updating"
    kill $(pgrep $binaryName)
    rm -rf $rundir.bak
    cp -r $rundir $rundir.bak
    
    cp -f "$workdir/build/"* "$rundir/"
    echo "Running main App"
    cd /
    cd $rundir
    ./$binaryName &
    sleep 10
    pgrep $binaryName
    pgr=$?
    echo "pgr=$pgr"
    if [ "$pgr" == "0" ]; then
    	currAppState="running"
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
    	currAppState="running"
        return 0
    else
    	currAppState="stopped"
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

checkAppRunning()
{
    

    #at first moment, just rum app if it is not running
    pgrep $binaryName
    pgr=$?
    echo "checkAppRunning pid=$pgr"
    if [ "$pgr" != "0" ]; then
        sendTelegram "App $binaryName is not running, starting it now!"
        cd $rundir
        ./$binaryName &
        sleep 10
        pgrep $binaryName
        if [ "$?" != "0" ]; then
            sendTelegram "⚠Hey! Failed to start app $binaryName⚠"
            currAppState="stopped"
        fi
    fi;

    while [ true ]; do
        pgrep $binaryName >/dev/null
        if [ "$?" == "0" ]; then
            #running
            if [ "$currAppState" == "stopped" ]; then
            	sendTelegram "👀the app $binaryName is running again !!👀" &
            	currAppState="running"
            fi
        else
            #stopped
            if [ "$currAppState" == "running" ]; then
                currAppState="stopped"
                sendTelegram "🆘the app $binaryName stopped running!!🆘\n👀" &
                sleep 1
                sendTelegram "Here go the last 100 lines of main log file" &
                sleep 1
                cat $mainLogFile | tail -n 100 > /tmp/last100logLines.log
                sendTelegramFile /tmp/last100logLines.log
            fi
        fi

        sleep 5;
    done;
}

main