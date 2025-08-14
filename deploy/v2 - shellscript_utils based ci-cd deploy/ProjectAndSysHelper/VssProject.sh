inherits "IProjectAnsSys" "$this->name"
saveFile=true
this->init(){
    new promise this->_initRetProm

    new "utils/utils.sh" "this->utils" 
    # arguments validation {
        #first arg is the logManager
        newRef "$1" "this->logManager"
        if [ $? -ne 0 ]; then 
            this->utils->derivateError2 "Error referencing logManager"; 
            this->_initRetProm->reject "$_error"; 
            _r="this->_initRetProm"
            return 1; 
        fi

        #second arg is the telegram object
        newRef "$2" "this->telegram"
        if [ $? -ne 0 ]; then 
            this->utils->derivateError2 "Error referencing telegram";
            this->_initRetProm->reject "$_error";
            _r="this->_initRetProm"
            return 1; 
        fi

        #third arg is the project repository
        this->projectRepoUrl="$3"
        if [ -z "$this->projectRepoUrl" ]; then
            _error="Project GIT repository not informed"; 
            this->_initRetProm->reject "$_error"; 
            _r="this->_initRetProm"
            return 1; 
        fi
    # }

    this->logManager->newNLog "deploy-system/VssHelper" "this->log"
    this->log->info "Starting the deploy script"

    this->workFolder="$(pwd)/workdata/VssProjectHelper"
    mkdir -p "$this->workFolder"
    this->projectDirectory=$this->workFolder/projectCode
    
    this->_initRetProm->resolve

    _r="this->_initRetProm"
    return 0
}

#this is not a pure function, as it uses the 'this->_initRetProm', that is created in the init function
this->_cloneRepo(){

    new promise this->_cloneProm

    rm -rf "$this->projectDirectory" > /dev/null 2>&1

    this->log->info "Cloning project "$this->projectRepoUrl" to the directory: $this->projectDirectory"
    this->log->debug "running 'command git clone --recursive \"$this->projectRepoUrl\" \"$this->projectDirectory\" > \"$this->workFolder/gitclone.log\" 2>&1"

    rm "$this->workFolder/cloneProjectDone" > /dev/null 2>&1
    rm "$this->workFolder/gitclone.log" > /dev/null 2>&1
    rm -rf "$this->workFolder/cloneProjectDone" > /dev/null 2>&1
    (
        git clone --recursive "$this->projectRepoUrl" "$this->projectDirectory" > "$this->workFolder/gitclone.log" 2>&1
        local retCode=$?
        echo "$retCode" > "$this->workFolder/cloneProjectDone"
    ) &

    scheduler->waitFor _(){
        #check
        if [ -f "$this->workFolder/cloneProjectDone" ]; then
            local retCode=$(cat "$this->workFolder/cloneProjectDone")
            if [ $retCode -ne 0 ]; then
                if [ -f "$this->workFolder/gitStderr.log" ]; then
                    _error=$(cat "$this->workFolder/gitStderr.log")
                else
                    _error="Unknown error. Git output: $(cat $this->workFolder/gitclone.log)"
                fi

                this->utils->derivateError2 "Error cloning the project"
            else
                _error=""
            fi

            #return zero to finalize the 'waitFor' operation
            return 0
        fi
        return 1
    }()_ _(){
        #on done
        if [ "$_error" != "" ]; then
            #this->log->error "$_error"
            
            this->log->info "cloneProject.log content: $(cat $this->workFolder/gitclone.log)"
            rm $this->workFolder/gitStdout.log

            this->_cloneProm->reject "$_error"
            return 1
        fi

        rm "$this->workFolder/cloneProjectDone" > /dev/null 2>&1
        rm "$this->workFolder/gitclone.log" > /dev/null 2>&1

        this->_cloneProm->resolve
        return 0
    }()_

    _r=this->_cloneProm
    return 0
}

this->checkout(){ this->__branch_or_tag="$1"; this->__chkout_onDone="$2"

    this->log->info "Removing the project directory: $this->projectDirectory"
    cd "$this->workFolder"
    rm -rf "$this->projectDirectory"

    this->log->info "Cloning the project repository"

    this->_cloneRepo
    newRef "$_r" this->_clonePromise

    this->_clonePromise->catch _(){
        _error="$1"
        this->utils->derivateError2 "Error cloring project"
        this->log->error "$_error"
        $this->__chkout_onDone 1 "$_error"
        finalizeRef "this->_clonePromise" true
    }()_
    
    this->_clonePromise->then _(){
        this->log->info "Project cloned successfully"
        finalizeRef "this->_clonePromise" true

        this->log->info "Checking out the project to the branch/tag: $this->__branch_or_tag"
        (
            cd "$this->projectDirectory"
            rm "$this->workFolder/checkoutDone" >/dev/null 2>&1
            rm "$this->workFolder/checkout.log" >/dev/null 2>&1

            git fetch > "$this->workFolder/checkout.log" 2>&1
            this->log->info "checking out the branch/tag: [$this->__branch_or_tag]"
            git checkout -f "$this->__branch_or_tag" >> "$this->workFolder/checkout.log" 2>&1
            local retCode=$?
            echo "$retCode" > "$this->workFolder/checkoutDone"
        ) &

        scheduler->waitFor _(){
            #check
            if [ -f "$this->workFolder/checkoutDone" ]; then
                local retCode=$(cat "$this->workFolder/checkoutDone")
                if [ $retCode -ne 0 ]; then
                    if [ -f "$this->workFolder/checkout.err" ]; then
                        _error=$(cat "$this->workFolder/checkout.err")
                    else
                        _error="Unknown error. Git output: $(cat $this->workFolder/checkout.log)"
                    fi

                    this->utils->derivateError2 "Error checking out the project"
                else
                    _error=""
                fi

                #return zero to finalize the 'waitFor' operation
                return 0
            fi
            return 1
        }()_ _(){
            #on done

            if [ "$_error" != "" ]; then
                this->log->error "$_error"
                this->log->info "git checkout stdout/stderr: $(cat $this->workFolder/checkout.log)"
                
                rm "$this->workFolder/checkoutDone" >/dev/null 2>&1
                rm "$this->workFolder/checkout.log" >/dev/null 2>&1

                $this->__chkout_onDone 1 "$_error"
                return 1
            fi

            rm "$this->workFolder/checkoutDone" >/dev/null 2>&1
            rm "$this->workFolder/checkout.log" >/dev/null 2>&1

            $this->__chkout_onDone 0
            return 0
        }()_

    }()_
}

# build, tests and deploy auxiliary functions {
    this->buildTests(){ this->__bldt_onDone="$1"
        this->log->info "----- [ Making tests ] -----"

        cd "$this->projectDirectory/tests"
        rm -rf build

        #run 'make all' with very low priority
        this->log->info "calling make all with nice -n 19"
        rm -rf "$this->workFolder/buildTestsDone"

        (nice -n 19 make all >make_tests.log 2>&1; echo $? > "$this->workFolder/buildTestsDone") &
        #( sleep 1; echo "ok" >make_tests.log 2>&1; echo $? > "$this->workFolder/buildTestsDone") &

        scheduler->runPeriodically _(){ 
            local tskId="$1"
            if [ -f "$this->workFolder/buildTestsDone" ]; then
                scheduler->erasePeriodic "$tskId"
                
                cd "$this->projectDirectory""/tests"

                this->log->info "command 'make all (on tests project)' finished"

                local buildTestsDone=$(cat "$this->workFolder/buildTestsDone")

                if [ $buildTestsDone -ne 0 ]; then
                    this->utils->derivateError2 "Error running make all (on tests project)"
                    this->log->error "$_error"
                    
                    this->log->info "make_tests.log content (loglevel $INFO):"
                    this->log->interceptCommandStdout "$COMMAND_TRACE" "cat ./make_tests.log"
                    
                    this->telegram->sendErrorMessage "Error building the tests: $_error"
                    this->telegram->sendFile "make_tests.log"
                    
                    rm "make_tests.log"
                    $this->__bldt_onDone 1 "$_error"
                    return 1
                fi
                eval "$this->__bldt_onDone" 0
                return 0
            fi
            return 1
        }()_ 1
    }

    this->runTests(){ this->__rnt_onDone="$1"
        
        this->log->info "----- [ Running tests ] -----"

        if [ ! -f "$this->projectDirectory/tests/build/tests" ]; then
            _error="tests binary not found"
            this->log->error "$_error"
            this->telegram->sendErrorMessage "$_error"
            return 1
        fi
        cd "$this->projectDirectory/tests/build"

        rm "$this->workFolder/runTestsDone" > /dev/null 2>&1
        this->log->info "calling ./tests"
        this->log->debug "running (nice -n 19 ./tests > tests.log 2>&1; echo $? > \"$this->workFolder/runTestsDone\") &"
        (nice -n 19 ./tests > tests.log 2>&1; echo $? > "$this->workFolder/runTestsDone") &

        this->log->debug "Now, is time to wait for the tests"

        #(sleep 1; echo "ok" > ./tests > tests.log 2>&1; echo $? > "$this->workFolder/runTestsDone") &
        local timeoutseconds=1800
        local checkIntervalSeconds=5
        scheduler->waitFor _(){
            if [ -f "$this->workFolder/runTestsDone" ]; then
                return 0
            fi
            return 1
        }()_ _(){
            this->log->info "command './tests' finished"
            local testRetCode=$(cat "$this->workFolder/runTestsDone")

            this->log->info "tests.log content:"

            #go back to the folder (another parts of the code may be using the same shell)
            cd "$this->projectDirectory/tests/build"
            this->log->info "$(cat ./tests.log)"
            
            if [ $testRetCode -ne 0 ]; then
                _error="Error running the tests"
                this->log->error "$_error"
                this->log->info "tests.log content: $(cat ./tests.log)"
                this->telegram->sendErrorMessage "$_error"
                this->telegram->sendFile "tests.log"

                this->__rnt_onDone 1 "$_error"
                return 1
            else
                this->log->info "Tests passed"
            fi
            rm "tests.log"
            $this->__rnt_onDone 0

        }()_ timeoutseconds checkIntervalSeconds
    }

    this->buildArtifacts(){ local customArtifactInfo=$1; this->__bldArt_onDone="$2"
        this->log->info "----- [ Making the project ] -----"
        cd "$this->projectDirectory"
        rm -rf build

        chmod +x ./makefile.aux/detect_include_files.sh
        this->log->debug "calling make all with nice -n 19"
        nice -n 19 make all >make.log 2>&1
        #sleep 1; echo "ok" >make.log 2>&1

        if [ $? -ne 0 ]; then
            _error="Error runing make all command"
            this->log->error "$_error"
            this->log->info "make.log content (loglevel $INFO):"
            #this->log->interceptCommandStdout "$COMMAND_TRACE" "cat ./make.log"
            this->log->info "make.log content: $(cat ./make.log)"
            this->telegram->sendErrorMessage "Error building the project"
            this->telegram->sendFile "make.log"
            this->__bldArt_onDone 1 "$_error"
            return 1
        else
            this->log->info "Project built successfully"
        fi

        $this->__bldArt_onDone 0
    }

    this->uploadArtifacts(){ local customArtifactInfo=$1; this->__uplArt_onDone="$2"
        local artifactsFolder="vss-$customArtifactInfo-build-artifacts-$(uname -m)"
        local destinationFile=""
        local commandResult=0

        cd "$this->projectDirectory"
        
        rm -rf build/objects/
        rm -rf $artifactsFolder
        cp -r build $artifactsFolder

        #check if zip is installed
        #if [ -z "$(which zip)" ]; then
        #    destinationFile="$artifactsFolder.zip"
        #    this->log->info "Creating the file: $destinationFile"
        #    this->log->interceptCommandStdout "$COMMAND_TRACE" "zip -r \"$destinationFile\" \"./$artifactsFolder\""
        #    commandResult=$?
        #else
            destinationFile="$artifactsFolder.tar.gz"
            rm -f "$destinationFile" > /dev/null 2>&1
            this->log->info "Creating the file: $destinationFile"
            this->log->interceptCommandStdout "$COMMAND_TRACE" "tar -czvf \"$destinationFile\" \"./$artifactsFolder\""
            commandResult=$?
        #fi
        
        if [ $commandResult -ne 0 ]; then
            _error="Error creating the zip file"
            this->log->error "$_error"
            $this->__uplArt_onDone 1 "$_error"
            return 1
        else
            this->log->info "Artifacts file created successfully"
            this->telegram->sendFile ./$destinationFile
        fi
        rm -rf "$artifactsFolder" > /dev/null 2>&1
        rm "$destinationFile" > /dev/null 2>&1

        $this->__uplArt_onDone 0
    }
# }


# vss replacement functions (delivery) {
    this->deployToProduction(){ local tag="$1"; local onDone="$2"; local state="$3"; local error="$4"
        echo "this->deployToProduction called with tag: $tag, state $state, error: $error and onDone: $onDone"
        this->log->debug "this->deployToProduction called with tag: $tag, state $state, error: $error and onDone: $onDone"

        if [ ! -z "$error" ]; then
            echo "entered in error case"
            this->utils->derivateError2 "Error replacing vss"
            this->log->error "$_error"
            eval "$onDone 1 \"$_error\""
            return 1
        fi


        echo 1
        echo "checking state: "
        if [ -z "$state" ]; then
            this->deployToProduction "$tag" "$onDone" "stopVssGuard"
        elif [ "$state" == "stopVssGuard" ]; then
            this->stopVssGuard "this->deployToProduction "$tag" "$onDone" "replaceFiles""
        elif [ "$state" == "replaceFiles" ]; then
            this->replaceVssBinaries "this->deployToProduction "$tag" "$onDone" "runVssGuard""
        elif [ "$state" == "runVssGuard" ]; then
            echo "running this->runVssGuard function"
            this->runVssGuard "this->deployToProduction "$tag" "$onDone" "done""
        elif [ "$state" == "done" ]; then
            this->log->info "Vss replaced"
            this->telegram->sendInfoMessage "Vss replaced in production"
            eval "$onDone 0"
        else
            echo "invalid state $state"
            if [ "$state" == "runVssGuard" ]; then
                echo "agora detectou"
            fi
        fi
    }

    this->stopVssGuard(){ this->tmpOnDone="$1";
        this->log->info "stopping vss guard"
        echo "stop" > ~/vss/control

        echo "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[calling wait for ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
        scheduler->waitFor _(){
            echo "checking"
            local vssGudartControlContent=$(cat ~/vss/control)
            if [ "$vssGudartControlContent" == "stopped" ]; then
                echo "Ok, vss guard stopped"
                return 0
            fi
            return 1
        }()_ _(){
            if [ "$_error" != "" ]; then
                echo "exiting with error"
                this->utils->derivateError2 "Error stopping vss guard"
                eval "$this->tmpOnDone \"$_error\""
            else
                echo "exiting with sucess"
                echo "evaluating '$this->tmpOnDone'"
                eval "$this->tmpOnDone"
            fi
        }()_ 10

    }

    this->replaceVssBinaries(){ this->_rvb_tmpOnDone="$1"

        this->log->info "replacing vss binary file"
        this->log->debug "running cp -r \"$this->projectDirectory/build/\"* ~/vss/bin/ > /tmp/vssreplbin 2>&1"
        cp -r "$this->projectDirectory/build/"* ~/vss/bin/ > /tmp/vssreplbin 2>&1
        if [ $? -ne 0 ]; then
            _error="$(cat /tmp/vssreplbin)"
            this->utils->derivateError2 "Error replacing vss binaries"
            eval "$this->_rvb_tmpOnDone \"$_error\""
            return 1
        fi
        eval "$this->_rvb_tmpOnDone" ""
        return 0
    }

    this->runVssGuard(){ this->_rvs_tmpOnDone="$1"
        echo "looooooooooooooooooooooooooooooooooooooooooooooooooooooool"
        this->log->info "Requesting vss start to the vss guard"
        #scheduler_waitFor(){ local checkCallback=$1; local doneCallback=$2; local _timeoutsec_=$3; local _taskCheckInterval_=$4
        echo "run" > ~/vss/control

        echo "calling w ait for"
        scheduler->waitFor _(){
            echo "checking vss guard status"
            local vssGudartControlContent=$(cat ~/vss/control)
            echo "current vss guart status: $vssGudartControlContent"
            this->log->debug "vssGudartControlContent = $vssGudartControlContent"
            if [ "$vssGudartControlContent" == "running" ]; then
                return 0
            fi
            return 1
        }()_ _(){
            echo "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee.. terminou!!!!"
            if [ "$_error" != "" ]; then
                this->utils->derivateError2 "Error running vss guard"
                this->log->error "$_error"
                eval "$this->_rvs_tmpOnDone \"$_error\""
            else
                this->log->info "Vss guard started"
                eval "$this->_rvs_tmpOnDone"
            fi
        }()_ 10
    }

# }

