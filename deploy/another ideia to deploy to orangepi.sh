#!/bin/bash


# SET CONFIGURATIONS VARIABLES

#CONSTANTS_AND_DEFAULTS()
# {
    LOG_LEVEL_DEBUG=0
    LOG_LEVEL_INFO=1
    LOG_LEVEL_WARNING=2
    LOG_LEVEL_ERROR=3

    ERROR_CODE_COMPILING=1
    ERROR_CODE_ONREMOTE_MOVE_TO_DESTINATION=3
    ERROR_CODE_ONREMOTE_STOPING_SERVICES=4
    ERROR_CODE_ONREMOTE_SWAPING_FOLDERS_NAMES=5
    ERROR_CODE_ONREMOTE_STARTING_SERVICES=6
    ERROR_CODE_ONREMOTE_CHECK_SERVICES_RUNNING=7
    ERROR_RUNNING_TESTS=8
    ERROR_CODE_COPYING_TO_REMOTE__MAKING_DIRECTORIES=9
    ERROR_CODE_COPYING_TO_REMOTE__COPYING_FILES=10
#}

#CONFIGURATIONS()
# {

    #name of the project
    PROJECT_NAME="homeaut2"

    #project folder
    PROJECT_FOLDER="/media/veracrypt/projects/rafinha_tonello/homeaut2"

    #specify the current log level
    CURRENT_LOG_LEVEL=$LOG_LEVEL_INFO

    #This programs monitor the last git commit of the 'main' branch and check if this commit
    #is the same of the last. This last commit is saved in the file bellow
    CURRENT_HASH_FILE="~/.cache/$PROJECT_NAME.githash"

    #this command will be executed int he project root folder (this command run in the remote machine)
    COMPILATION_COMMAND="make ."
#}

GET_REMOTE_TMP_PROJECT_FOLDER()
{
    return /home/${USERNAME}/.cache/${projectName}
}

CONSTANTS_AND_DEFAULTS()
{
    

    #this is the default return flag for functions. Remember: bash functions should return errors codes.
    _return=""
}


# the main function
# arguments:
#   remoteMachineIp ($1): the ip of the remote machine
#   remoteMachineUsername ($2): The username to be used with sudo command in the remote machine
#   remoteMachinePassword ($3): the passwor of the remoteMachineUsername
MAIN()
{
    local remoteIp=$1
    local remoteUsername=$2
    local remotePassword=$3

    #CONSTANTS_AND_DEFAULTS
    #CONFIGURATIONS

    monitor_git_and_deploy "$remoteIp" "$remoteUsername" "$remotePassword" "$PROJECT_NAME" "$PROJECT_FOLDER" "$CURRENT_HASH_FILE"
    return $?
}

# the default function to print data to terminal and send desktop-notifications
# arguments:
#   message ($1): The message to be displayed
#   messageLevel ($2): The level of the message. If the level is bigger of equal to CURRENT_LOG_LEVEL, the message will be displayed
#   sentToNotifySend ($3): This parameter indicates if the message should be send to desktop notification system (gnome)
log()
{
    message=$1
    logLevel=$LOG_LEVEL_INFO
    if [ ! -z $2 ]
    then
        logLevel=$2
    fi

    sendToNotifySend=0
    if [ ! -z $3 ]
    then
        sendToNotifySend=1
    fi

    if [ $logLevel -ge $CURRENT_LOG_LEVEL ]
    then
        echo "$message"
    fi

    if [[ "1trueTRUE" == *"$sendToNotifySend"* ]]
    then
        if [ $logLevel -ge $LOG_LEVEL_WARNING ]
        then
            notify-send --urgency=critical "$message"
        else
            notify-send "$message"
        fi
    fi
}

# Converts a error code to a string
# arguments:
#   errorCode($1): The error code to be converted
errorCodeToString()
{
    local errorCode=$1
    if [ "$errorCode" == "$ERROR_CODE_COMPILING" ]
    then
        _return="Error compiling the project (in the remote machine)"
    elif [ "$errorCode" == "$ERROR_CODE_ONREMOTE_MOVE_TO_DESTINATION" ]
    then
        _return="Error moving binary files to destination (in the remote machine)"
    elif [ "$errorCode" == "$ERROR_CODE_ONREMOTE_STOPING_SERVICES" ]
    then
        _return="Error stoping services (in the remote machine) "
    elif [ "$errorCode" == "$ERROR_CODE_ONREMOTE_SWAPING_FOLDERS_NAMES" ]
    then
        _return="Error swaping folders names (in the remote machine)"
    elif [ "$errorCode" == "$ERROR_CODE_ONREMOTE_STARTING_SERVICES" ]
    then
        _return="Error starting services (in the remote machine)"
    elif [ "$errorCode" == "$ERROR_CODE_ONREMOTE_CHECK_SERVICES_RUNNING" ]
    then
        _return="Services are note running in the remote machine"
    elif [ "$errorCode" == "$ERROR_RUNNING_TESTS" ]
    then
        _return="Error running tests in the remote machine"
    elif [ "$errorCode" == "$ERROR_CODE_COPYING_TO_REMOTE__MAKING_DIRECTORIES" ]
    then
        _return="Error during copying files to remote machine (cannot create directories)"
    elif [ "$errorCode" == "$ERROR_CODE_COPYING_TO_REMOTE__COPYING_FILES" ]
    then
        _return="Error during copying files to remote machine"

    else
        _return="Unknown error (code: $errorCode)"
    fi
    

    return 0;
}

# monitores a git project and detect if a deploy must be runned
# arguments
#   remoteMachineIp ($1)
#   remoteMachineUsername ($2)
#   remoteMachinePassword ($3)
#   projectName ($4)
#   projectFolder ($5): The project folder
#   hashFile ($6): the hash file when the last commit hash is stored (see the CONFIGURATIONS function)
monitor_git_and_deploy()
{
    local remoteIp=$1
    local remoteUsername=$2
    local remotePassword=$3
    local projectName=$4
    local projectFolder=$5
    local hashFile=$6
    local lastDeployHash=""
    local currentHash=""
    while [ true ]
    do
        log "starting a new deploy check" $LOG_LEVEL_DEBUG

        readLastDeployHash $hashFile
        lastDeployHash=$_return
        
        getCurrentHash $projectFolder
        currentHash=$_return

        log "currentHash: $currentHash" $LOG_LEVEL_DEBUG
        log "lastDeployHash: $lastDeployHash" $LOG_LEVEL_DEBUG

        if [ "$lastDeployHash" != "$currentHash" ]
        then
            log "staring deploy" $LOG_LEVEL_INFO 1
            deploy "$remoteIp" "$remoteUsername" "$remotePassword" "$projectName" "$projectFolder"

            if [ "$?" == "0"]
            then
                saveLastDeployHash $hashFile $currentHash
                log "Deploy done!" $LOG_LEVEL_INFO 1
            else
                errorCodeToString $?
                local errorDescription=$_return
                log "Deploy error: $errorDescription" $LOG_LEVEL_ERROR 1
            fi
        fi

        sleep 10
    done

    return 0
}

# load the last commit hash from the hash file
# aruments:
#   hashFile ($1): the hash file
readLastDeployHash()
{
    filename=$1
    _return=$(cat $filename)
    return 0
}

# saves the last commit hash to the hash file
# arguments:
#   filename ($1): the hash file
#   hash ($2): the hash of the last commit of the 'main' branch
saveLastDeployHash()
{
    local filename=$1
    local content=$2
    echo "$content" > "$filename"
    return 0
}


# Get the last commit of the 'main' branch
# arguments
#   projectFolder ($1): the project root folder
getCurrentHash()
{
    local projectFolder=$1
    local currentFolder=$(pwd)
    cd $projectFolder
    _return=$(git log -n 1 main --pretty=format:"%H")
    cd $currentFolder
    return 0
}

# orchestrate the deployment (run the deploy pipeline). This function is called when a new commit is done in the 'main' branch of the project
# arguments:
#   remoteMachineIp ($1): the remote machine ip
#   username ($2): the root username
#   password ($3): the password of the root user
#   projectName ($5): the temporary project folder in the remote machine
#   projectFolder ($4): the project folder
deploy()
{
    local machineIp=$1
    local username=$2
    local password=$3
    local projectName=$4
    local projectFolder=$5
    local projectFolderOnRemoteMachine="/home/${username}/.local/$projectName"

    copyToRemote $machineIp $username $password $projectFolder $projectFolderOnRemoteMachine
    local errorCode=$?
    if [ "$errorCode" != "0" ]
    then
        onRemote_clearFiles
        return $errorCode
    fi

    onRemote_runTests "$machineIp" "$username" "$password"
    if [ "$?" != "0" ]
    then
        onRemote_clearFiles
        reportTestsErrors
        return $ERROR_RUNNING_TESTS
    fi

    onRemote_compile
    if [ "$?" != "0" ]
    then
        onRemote_clearFiles
        return $ERROR_CODE_COMPILING
    fi

    onRemote_moveBinariesToDestination
    if [ "$?" != "0" ]
    then
        onRemote_clearFiles
        return $ERROR_CODE_ONREMOTE_MOVE_TO_DESTINATION
    fi

    onRemote_stop
    if [ "$?" != "0" ]
    then
        onRemote_clearFiles
        return $ERROR_CODE_ONREMOTE_STOPING_SERVICES
    fi

    onRemote_swapFolderNames
    if [ "$?" != "0" ]
    then
        rollback
        onRemote_clearFiles
        return $ERROR_CODE_ONREMOTE_SWAPING_FOLDERS_NAMES
    fi

    onRemote_start
    if [ "$?" != "0" ]
    then
        rollback
        onRemote_clearFiles
        return $ERROR_CODE_ONREMOTE_STARTING_SERVICES
    fi

    onRemote_check_if_services_running
    if [ "$?" != "0" ]
    then
        rollback
        onRemote_clearFiles
        return $ERROR_CODE_ONREMOTE_CHECK_SERVICES_RUNNING
    fi

    return 0
}

#this function is called when de deploy finds any error
rollback()
{
    log "Rollbacking the deploy" $LOG_LEVEL_WARNING 1
    onRemote_stop_rollback
    onRemote_swapFolderNames_rollback
    onRemote_start
}


# Copy project sources to the destination machine
# arguments:
#   remoteMachineIp ($1): the remote machine ip
#   username ($2): the root username
#   password ($3): the password of the root user
#   projectFolder ($4): the project folder
#   tempFolderOnRemoteMachine ($5): the temporary project folder in the remote machine
copyToRemote()
{
    local SERVERIP=$1
    local USERNAME=$2
    local PASSWORD=$3
    local PROJECTFOLDER=$4
    local TMP_PROJECT_FOLDER_ON_REMOTE_MACHINE=$5

    RUN_ON_REMOTE $SERVERIP $USERNAME $PASSWORD "mkdir $TMP_PROJECT_FOLDER_ON_REMOTE_MACHINE"
    if [ "$?" != "0" ]
    then
        return ERROR_CODE_COPYING_TO_REMOTE__MAKING_DIRECTORIES
    fi
    
    eval "sshpass -p ${PASSWORD} scp -o \"StrictHostKeyChecking no\" -r $PROJECTFOLDER/* ${USERNAME}@${SERVERIP}:$TMP_PROJECT_FOLDER_ON_REMOTE_MACHINE"
    if [ "$?" != "0" ]
    then
        return ERROR_CODE_COPYING_TO_REMOTE__COPYING_FILES
    fi
}

onRemote_runTests()
{
    :
}
    
reportTestsErrors()
{
    :
}


# run the compilation command in the remote machine
# arguments:
#   remoteMachineIp ($1): the remote machine ip
#   username ($2): the root username
#   password ($3): the password of the root user
#   projectTmpFolder ($4): the temporary project folder in the remote machine
# Notes:
#   This is note a pure function: compilation command comes from a global var
onRemote_compile()
{
    RUN_ON_REMOTE $1 $2 $3 "cd $4; $COMPILATION_COMMAND"
}

onRemote_moveBinariesToDestination()
{
    :
}

onRemote_stop()
{
    :
}

onRemote_swapFolderNames()
{
    :
}

onRemote_start()
{
    :
}

onRemote_check_if_services_running()
{
    :
}

onRemote_clearFiles()
{
    :
}

#argument machineIp: the machine ip address
#argument username: the root username
#argument password: the password of the root user
#argument command: the command to be executed
RUN_ON_REMOTE()
{
    #cmd="sshpass -p $CFG_SSH_PASSWORD ssh -o LogLevel=QUIET -o \"StrictHostKeyChecking no\" -t $CFG_SSH_USERNAME@$serverIp \"echo $CFG_SSH_PASSWORD | sudo -S cat /sys/class/dmi/id/product_uuid\""
    local serverip=$1
    local username=$2
    local password=$3
    local command=$4
    #local cmd="sshpass -p $password ssh -o LogLevel=QUIET -o \"StrictHostKeyChecking no\" -t $username@$serverip \"$command\""
    local cmd="sshpass -p $password ssh ${DEFAULT_SSH_ARGS} -o \"StrictHostKeyChecking no\" -t $username@$serverip \"$command\""
    eval $cmd
}

export -f RUN_ON_REMOTE

#argument machineIp: the machine ip address
#argument username: the root username
#argument password: the password of the root user
#argument command: the command to be executed
RUN_ON_REMOTE_AS_ROOT()
{
    local serverip=$1
    local username=$2
    local password=$3
    local command=$4
    #local cmd="sshpass -p $password ssh -o LogLevel=QUIET -o \"StrictHostKeyChecking no\" -t $username@$serverip \"echo $password | sudo -S $command\""
    local cmd="sshpass -p $password ssh ${DEFAULT_SSH_ARGS} -o \"StrictHostKeyChecking no\" -t $username@$serverip \"echo $password | sudo -S $command\""
    eval $cmd
}


MAIN $1 $2 $3





