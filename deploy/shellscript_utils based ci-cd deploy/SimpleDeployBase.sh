#!/bin/bash

# TODO: move to Shell Script Utils Project

if [ "$1" != "inheritance" ]; then 
    echo "This class should be inherited by another class. Use the command 'inherit' to do so. Do not forget to ovrride the methods 'defaultPipe' and 'releasePipe'"
    exit 1
fi

this->init(){ local ProjectName="$1"; local repositoryUrl="$2"; local _auto_start_gitObserver_def_true="$3"
    this->ProjectName="$ProjectName"
    this->repositoryUrl="$repositoryUrl"

    if [ -z "$_auto_start_gitObserver_def_true" ]; then
        _auto_start_gitObserver_def_true=true
    fi

    #configurations
    _this->logFile="$(pwd)/logfile.log"
    #_this->tlgBotKey=$TELEGRAM_BOT_KEY
    #_this->tlgChatId=$TELEGRAM_CHAT_ID

    #these variables are used by instances of sharedmemory to create directories. Sharedmemory class is used as a keyvalue database
    this->databseNamespace="deploy"
    this->databaseDirectory="$(pwd)/workdata"

    #log setup
    new "logterminalwriter" "this->logTerminalWriter" "$INFO"
    new "logfilewriter" "this->logFileWriter" "$_this->logFile" 0
    new "logger" "this->logManager" 1 1 0 "this->logTerminalWriter" "this->logFileWriter"

    new "utils/strutils" "this->strutils"

    this->logManager->newNLog "deploy-system" "this->log"
    this->log->info "Starting the deploy script"

    #CREATE A CUSTOM LOG LEVEL FOR COMMAND INTERCEPTION
    createLogLevel "COMMAND_TRACE" 25 '\033[0;34m'
    
    #creates a new queue to schedule the pipes
    new "persisqueue.sh" this->queue "deployqueue" "$this->databaseDirectory"
    
    #creates a new telegram object to send messages, informing the status of the pipes, error, etc
    this->log->debug "creating telegram object"

    if $_auto_start_gitObserver_def_true; then
        this->startupGitObserver
    fi

    this->_waitingPreviousOperation=false

    scheduler->runPeriodically "this->consumeNextQueueElement" 5 1
}

# setup {
    this->startupGitObserver(){

        #creates a new git observer, to watch what is going on in the git repository
        new "gitobserver" "this->gitObserver" \
            "testproject" \
            "$this->repositoryUrl" \
            "" \
            "$this->databaseDirectory" \
            "" \
            "this->logManager" \
            120


        #this->gitObserver->onCommit->subscribe "__f(){ local commit=\$1
        #    this->log->info \"New commit: \$commit. Enqueueing the commit to be processed\"
        #    this->queue->pushBack \"gitcommit\" \"\$commit\"
        #}; __f" 


        this->gitObserver->onTag->subscribe _(){ local tag="$1"; commit="$2"; commitMessage="$3"; commitAuthor="$4" commitDate="$5"
            this->log->info "New tag: $tag. Enqueueing the tag to be processed"
            echo "-------------------------- enqueuing gittag"
            this->queue->pushBack "gittag" "$tag" "$commit" "$commitMessage" "$commitAuthor" "$commitDate"
        }()_

        this->gitObserver->onBranchNewCommits->subscribe _(){ newRef "$1" commitsInfo
            this->log->info "New commits in branch $commitsInfo->branchName. Last commit is $commitsInfo->lastCommit. Enqueueing the commit to be processed"
            
            serializeObject "commitsInfo"
            
            echo "----------------------------- enqueuing gitcommit"
            this->queue->pushBack "gitcommit" "$_r"
        }()_
    }
#}

# pipes {
    #default pipe (the continious integration), runs the tests when a new commit is pushed to the 
    #repository. Receives a gitobserver::StructBranchCommits object
    #
    #Should returns a promise
    this->defaultPipe(){ local structBranchCommits=$1; onDone="$2"
        this->log->error "Not implemented. This method (defaultPipe) must be overriden"
        eval "$onDone"
    }

    #when a new tag is pushed to the repository, this pipe is executed, it runs the tests and
    #release the new version
    #
    #Should returns a promise
    this->releasePipe(){ local tag=$1; onDone="$2"
        this->log->error "Not implemented. This method (releasePipe) must be overriden"
        eval "$onDone"
    }
#}

# operations and functions {
    
    this->consumeNextQueueElement(){

        if $this->_waitingPreviousOperation; then
            return
        fi

        this->queue->popFront

        local size=${#_r[@]}
        if [ $size -eq 0 ]; then
            return 1
        fi
        
        local type=${_r[1]}
        #remove first _r element

        echo "--------------------------------------------------- type: $type"

        case $type in
            "gitcommit")
                this->_waitingPreviousOperation=true

                
                #display _r content (_r is an array)
                this->defaultPipe "${_r[2]}" 
                newRef "$_r" "this->_cnqe_prom"

                if [ "$this->_cnqe_prom->class" == "promise" ]; then
                    this->_cnqe_prom->then _(){
                        echo "--------------------------------------------------- git commit sucess"
                        finalizeRef "this->_cnqe_prom" true
                        this->_waitingPreviousOperation=false
                        this->consumeNextQueueElement
                    }()_
                    this->_cnqe_prom->catch _(){
                        echo "--------------------------------------------------- git commit error"
                        finalizeRef "this->_cnqe_prom" true
                        this->_waitingPreviousOperation=false
                        this->consumeNextQueueElement
                    }()_
                    return 0
                else
                    _error="Error: the defaultPipe method should return a promise (via _r variable)"
                    this->log->error "$_error"
                    this->log->warning "The call for default pipe will be ignored and the next element in the queue will be consumed"
                    return 1
                fi 

                
                ;;
            "gittag")
                this->_waitingPreviousOperation=true

                this->releasePipe "${_r[2]}" "${_r[3]}" "${_r[4]}"
                newRef "$_r" "this->_cnqe_prom2"

                if [ "$this->_cnqe_prom2->class" == "promise" ]; then
                    this->_cnqe_prom2->then _(){
                        echo "--------------------------------------------------- git tag sucess"
                        finalizeRef "this->_cnqe_prom2" true
                        this->_waitingPreviousOperation=false
                        this->consumeNextQueueElement
                    }()_
                    this->_cnqe_prom2->catch _(){
                        echo "--------------------------------------------------- git tag error"
                        finalizeRef "this->_cnqe_prom2" true
                        this->_waitingPreviousOperation=false
                        this->consumeNextQueueElement
                    }()_
                else
                    _error="Error: the releasePipe method should return a promise (via _r variable)"
                    this->log->error "$_error"
                    this->log->warning "The call for release pipe will be ignored and the next element in the queue will be consumed"
                    return 1
                fi

                return 0
                ;;
        esac

        return 1
    }
#}
