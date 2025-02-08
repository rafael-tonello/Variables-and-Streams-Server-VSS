inherits "IProjectAnsSys" "$__app___ProjAndSysHelper_name"
saveFile=true
__app___ProjAndSysHelper_init(){
    new promise __app___ProjAndSysHelper__initRetProm

    new "utils/utils.sh" "__app___ProjAndSysHelper_utils" 
    # arguments validation {
        #first arg is the logManager
        newRef "$1" "__app___ProjAndSysHelper_logManager"
        if [ $? -ne 0 ]; then 
            __app___ProjAndSysHelper_utils_derivateError2 "Error referencing logManager"; 
            __app___ProjAndSysHelper__initRetProm_reject "$_error"; 
            _r="__app___ProjAndSysHelper__initRetProm"
            return 1; 
        fi

        #second arg is the telegram object
        newRef "$2" "__app___ProjAndSysHelper_telegram"
        if [ $? -ne 0 ]; then 
            __app___ProjAndSysHelper_utils_derivateError2 "Error referencing telegram";
            __app___ProjAndSysHelper__initRetProm_reject "$_error";
            _r="__app___ProjAndSysHelper__initRetProm"
            return 1; 
        fi

        #third arg is the project repository
        __app___ProjAndSysHelper_projectRepoUrl="$3"
        if [ -z "$__app___ProjAndSysHelper_projectRepoUrl" ]; then
            _error="Project GIT repository not informed"; 
            __app___ProjAndSysHelper__initRetProm_reject "$_error"; 
            _r="__app___ProjAndSysHelper__initRetProm"
            return 1; 
        fi
    # }

    __app___ProjAndSysHelper_logManager_newNLog "deploy-system/VssHelper" "__app___ProjAndSysHelper_log"
    __app___ProjAndSysHelper_log_info "Starting the deploy script"

    __app___ProjAndSysHelper_workFolder="$(pwd)/workdata/VssProjectHelper"
    mkdir -p "$__app___ProjAndSysHelper_workFolder"
    __app___ProjAndSysHelper_projectDirectory=$__app___ProjAndSysHelper_workFolder/projectCode
    
    __app___ProjAndSysHelper__initRetProm_resolve

    _r="__app___ProjAndSysHelper__initRetProm"
    return 0
}

#this is not a pure function, as it uses the '__app___ProjAndSysHelper__initRetProm', that is created in the init function
__app___ProjAndSysHelper__cloneRepo(){

    new promise __app___ProjAndSysHelper__cloneProm

    rm -rf "$__app___ProjAndSysHelper_projectDirectory" > /dev/null 2>&1

    __app___ProjAndSysHelper_log_info "Cloning project "$__app___ProjAndSysHelper_projectRepoUrl" to the directory: $__app___ProjAndSysHelper_projectDirectory"
    __app___ProjAndSysHelper_log_debug "running 'command git clone --recursive \"$__app___ProjAndSysHelper_projectRepoUrl\" \"$__app___ProjAndSysHelper_projectDirectory\" > \"$__app___ProjAndSysHelper_workFolder/gitclone.log\" 2>&1"

    rm "$__app___ProjAndSysHelper_workFolder/cloneProjectDone" > /dev/null 2>&1
    rm "$__app___ProjAndSysHelper_workFolder/gitclone.log" > /dev/null 2>&1
    rm -rf "$__app___ProjAndSysHelper_workFolder/cloneProjectDone" > /dev/null 2>&1
    (
        git clone --recursive "$__app___ProjAndSysHelper_projectRepoUrl" "$__app___ProjAndSysHelper_projectDirectory" > "$__app___ProjAndSysHelper_workFolder/gitclone.log" 2>&1
        local retCode=$?
        echo "$retCode" > "$__app___ProjAndSysHelper_workFolder/cloneProjectDone"
    ) &

    scheduler_waitFor anonimousFunction_20 anonimousFunction_21

    _r=__app___ProjAndSysHelper__cloneProm
    return 0
}

__app___ProjAndSysHelper_checkout(){ __app___ProjAndSysHelper___branch_or_tag="$1"; __app___ProjAndSysHelper___chkout_onDone="$2"

    __app___ProjAndSysHelper_log_info "Removing the project directory: $__app___ProjAndSysHelper_projectDirectory"
    cd "$__app___ProjAndSysHelper_workFolder"
    rm -rf "$__app___ProjAndSysHelper_projectDirectory"

    __app___ProjAndSysHelper_log_info "Cloning the project repository"

    __app___ProjAndSysHelper__cloneRepo
    newRef "$_r" __app___ProjAndSysHelper__clonePromise

    __app___ProjAndSysHelper__clonePromise_catch anonimousFunction_22
    
    __app___ProjAndSysHelper__clonePromise_then anonimousFunction_23
}

# build, tests and deploy auxiliary functions {
    __app___ProjAndSysHelper_buildTests(){ __app___ProjAndSysHelper___bldt_onDone="$1"
        __app___ProjAndSysHelper_log_info "----- [ Making tests ] -----"

        cd "$__app___ProjAndSysHelper_projectDirectory/tests"
        rm -rf build

        #run 'make all' with very low priority
        __app___ProjAndSysHelper_log_info "calling make all with nice -n 19"
        rm -rf "$__app___ProjAndSysHelper_workFolder/buildTestsDone"

        (nice -n 19 make all >make_tests.log 2>&1; echo $? > "$__app___ProjAndSysHelper_workFolder/buildTestsDone") &
        #( sleep 1; echo "ok" >make_tests.log 2>&1; echo $? > "$__app___ProjAndSysHelper_workFolder/buildTestsDone") &

        scheduler_runPeriodically anonimousFunction_24 1
    }

    __app___ProjAndSysHelper_runTests(){ __app___ProjAndSysHelper___rnt_onDone="$1"
        __app___ProjAndSysHelper_log_info "----- [ Running tests ] -----"

        if [ ! -f "$__app___ProjAndSysHelper_projectDirectory/tests/build/tests" ]; then
            _error="tests binary not found"
            __app___ProjAndSysHelper_log_error "$_error"
            __app___ProjAndSysHelper_telegram_sendErrorMessage "$_error"
            return 1
        fi
        cd "$__app___ProjAndSysHelper_projectDirectory/tests/build"

        rm "$__app___ProjAndSysHelper_workFolder/runTestsDone" > /dev/null 2>&1
        __app___ProjAndSysHelper_log_info "calling ./tests"
        __app___ProjAndSysHelper_log_debug "running (nice -n 19 ./tests > tests.log 2>&1; echo $? > \"$__app___ProjAndSysHelper_workFolder/runTestsDone\") &"
        (nice -n 19 ./tests > tests.log 2>&1; echo $? > "$__app___ProjAndSysHelper_workFolder/runTestsDone") &

        __app___ProjAndSysHelper_log_debug "Now, is time to wait for the tests"

        #(sleep 1; echo "ok" > ./tests > tests.log 2>&1; echo $? > "$__app___ProjAndSysHelper_workFolder/runTestsDone") &
        local timeoutseconds=1800
        local checkIntervalSeconds=5
        scheduler_waitFor anonimousFunction_25 anonimousFunction_26 timeoutseconds checkIntervalSeconds
    }

    __app___ProjAndSysHelper_buildArtifacts(){ local customArtifactInfo=$1; __app___ProjAndSysHelper___bldArt_onDone="$2"
        __app___ProjAndSysHelper_log_info "----- [ Making the project ] -----"
        cd "$__app___ProjAndSysHelper_projectDirectory"
        rm -rf build

        chmod +x ./makefile.aux/detect_include_files.sh
        __app___ProjAndSysHelper_log_debug "calling make all with nice -n 19"
        nice -n 19 make all >make.log 2>&1
        #sleep 1; echo "ok" >make.log 2>&1

        if [ $? -ne 0 ]; then
            _error="Error runing make all command"
            __app___ProjAndSysHelper_log_error "$_error"
            __app___ProjAndSysHelper_log_info "make.log content (loglevel $INFO):"
            #__app___ProjAndSysHelper_log_interceptCommandStdout "$COMMAND_TRACE" "cat ./make.log"
            __app___ProjAndSysHelper_log_info "make.log content: $(cat ./make.log)"
            __app___ProjAndSysHelper_telegram_sendErrorMessage "Error building the project"
            __app___ProjAndSysHelper_telegram_sendFile "make.log"
            __app___ProjAndSysHelper___bldArt_onDone 1 "$_error"
            return 1
        else
            __app___ProjAndSysHelper_log_info "Project built successfully"
        fi

        $__app___ProjAndSysHelper___bldArt_onDone 0
    }

    __app___ProjAndSysHelper_uploadArtifacts(){ local customArtifactInfo=$1; __app___ProjAndSysHelper___uplArt_onDone="$2"
        local artifactsFolder="vss-$customArtifactInfo-build-artifacts-$(uname -m)"
        local destinationFile=""
        local commandResult=0

        cd "$__app___ProjAndSysHelper_projectDirectory"
        
        rm -rf build/objects/
        rm -rf $artifactsFolder
        cp -r build $artifactsFolder

        #check if zip is installed
        #if [ -z "$(which zip)" ]; then
        #    destinationFile="$artifactsFolder.zip"
        #    __app___ProjAndSysHelper_log_info "Creating the file: $destinationFile"
        #    __app___ProjAndSysHelper_log_interceptCommandStdout "$COMMAND_TRACE" "zip -r \"$destinationFile\" \"./$artifactsFolder\""
        #    commandResult=$?
        #else
            destinationFile="$artifactsFolder.tar.gz"
            rm -f "$destinationFile" > /dev/null 2>&1
            __app___ProjAndSysHelper_log_info "Creating the file: $destinationFile"
            __app___ProjAndSysHelper_log_interceptCommandStdout "$COMMAND_TRACE" "tar -czvf \"$destinationFile\" \"./$artifactsFolder\""
            commandResult=$?
        #fi
        
        if [ $commandResult -ne 0 ]; then
            _error="Error creating the zip file"
            __app___ProjAndSysHelper_log_error "$_error"
            $__app___ProjAndSysHelper___uplArt_onDone 1 "$_error"
            return 1
        else
            __app___ProjAndSysHelper_log_info "Artifacts file created successfully"
            __app___ProjAndSysHelper_telegram_sendFile ./$destinationFile
        fi
        rm -rf "$artifactsFolder" > /dev/null 2>&1
        rm "$destinationFile" > /dev/null 2>&1

        $__app___ProjAndSysHelper___uplArt_onDone 0
    }
# }


# vss replacement functions (delivery) {
    __app___ProjAndSysHelper_deployToProduction(){ local tag="$1"; local onDone="$2"; local state="$3"; local error="$4"
        __app___ProjAndSysHelper_log_debug "__app___ProjAndSysHelper_deployToProduction called with tag: $tag, state $state, error: $error and onDone: $onDone"

        if [ ! -z "$error" ]; then
            __app___ProjAndSysHelper_utils_derivateError2 "Error replacing vss"
            __app___ProjAndSysHelper_log_error "$_error"
            eval "$onDone 1 \"$_error\""
            return 1
        fi
        
        if [ -z "$state" ]; then
            __app___ProjAndSysHelper_deployToProduction "$tag" "$onDone" "stopVssGuard"
        elif [ "$state" == "stopVssGuard" ]; then
            __app___ProjAndSysHelper_stopVssGuard "__app___ProjAndSysHelper_deployToProduction "$tag" "$onDone" "replaceFiles""
        elif [ "$state" == "replaceFiles" ]; then
            __app___ProjAndSysHelper_replaceVssBinaries "__app___ProjAndSysHelper_deployToProduction "$tag" "$onDone" "runVssGuard""
        elif [ "$state" == "runVssGuard" ]; then
            __app___ProjAndSysHelper_runVssGuard "__app___ProjAndSysHelper_deployToProduction "$tag" "$onDone" "done""
        elif [ "$state" == "done" ]; then
            __app___ProjAndSysHelper_log_info "Vss replaced"
            __app___ProjAndSysHelper_telegram_sendInfoMessage "Vss replaced in production"
            eval "$onDone 0"
        else
            _error=VssProject::deployToProduction called with an invalid state: $state
            __app___ProjAndSysHelper_log_error "$_error"
            eval "$onDone 1 \"$_error\""
        fi
    }

    __app___ProjAndSysHelper_stopVssGuard(){ __app___ProjAndSysHelper_tmpOnDone="$1";
        __app___ProjAndSysHelper_log_info "stopping vss guard"
        echo "stop" > ~/vss/control

        scheduler_waitFor anonimousFunction_27 anonimousFunction_28 10

    }

    __app___ProjAndSysHelper_replaceVssBinaries(){ __app___ProjAndSysHelper__rvb_tmpOnDone="$1"
        __app___ProjAndSysHelper_log_info "replacing vss binary file"
        __app___ProjAndSysHelper_log_debug "running cp -r \"$__app___ProjAndSysHelper_projectDirectory/build/\"* ~/vss/bin/ > /tmp/vssreplbin 2>&1"
        cp -r "$__app___ProjAndSysHelper_projectDirectory/build/"* ~/vss/bin/ > /tmp/vssreplbin 2>&1
        if [ $? -ne 0 ]; then
            _error="$(cat /tmp/vssreplbin)"
            __app___ProjAndSysHelper_utils_derivateError2 "Error replacing vss binaries"
            eval "$__app___ProjAndSysHelper__rvb_tmpOnDone \"$_error\""
            return 1
        fi
        eval "$__app___ProjAndSysHelper__rvb_tmpOnDone" ""
        return 0
    }

    __app___ProjAndSysHelper_runVssGuard(){ __app___ProjAndSysHelper__rvs_tmpOnDone="$1"
        __app___ProjAndSysHelper_log_info "Requesting vss start to the vss guard"

        echo "run" > ~/vss/control

        scheduler_waitFor anonimousFunction_29 anonimousFunction_30 10
    }

# }

function anonimousFunction_20(){ 
        #check
        if [ -f "$__app___ProjAndSysHelper_workFolder/cloneProjectDone" ]; then
            local retCode=$(cat "$__app___ProjAndSysHelper_workFolder/cloneProjectDone")
            if [ $retCode -ne 0 ]; then
                if [ -f "$__app___ProjAndSysHelper_workFolder/gitStderr.log" ]; then
                    _error=$(cat "$__app___ProjAndSysHelper_workFolder/gitStderr.log")
                else
                    _error="Unknown error. Git output: $(cat $__app___ProjAndSysHelper_workFolder/gitclone.log)"
                fi

                __app___ProjAndSysHelper_utils_derivateError2 "Error cloning the project"
            else
                _error=""
            fi

            #return zero to finalize the 'waitFor' operation
            return 0
        fi
        return 1
     }

function anonimousFunction_21(){ 
        #on done
        if [ "$_error" != "" ]; then
            #__app___ProjAndSysHelper_log_error "$_error"
            
            __app___ProjAndSysHelper_log_info "cloneProject.log content: $(cat $__app___ProjAndSysHelper_workFolder/gitclone.log)"
            rm $__app___ProjAndSysHelper_workFolder/gitStdout.log

            __app___ProjAndSysHelper__cloneProm_reject "$_error"
            return 1
        fi

        rm "$__app___ProjAndSysHelper_workFolder/cloneProjectDone" > /dev/null 2>&1
        rm "$__app___ProjAndSysHelper_workFolder/gitclone.log" > /dev/null 2>&1

        __app___ProjAndSysHelper__cloneProm_resolve
        return 0
     }

function anonimousFunction_22(){ 
        _error="$1"
        __app___ProjAndSysHelper_utils_derivateError2 "Error cloring project"
        __app___ProjAndSysHelper_log_error "$_error"
        $__app___ProjAndSysHelper___chkout_onDone 1 "$_error"
        finalizeRef "__app___ProjAndSysHelper__clonePromise" true
     }

function anonimousFunction_23(){ 
        __app___ProjAndSysHelper_log_info "Project cloned successfully"
        finalizeRef "__app___ProjAndSysHelper__clonePromise" true

        __app___ProjAndSysHelper_log_info "Checking out the project to the branch/tag: $__app___ProjAndSysHelper___branch_or_tag"
        (
            cd "$__app___ProjAndSysHelper_projectDirectory"
            rm "$__app___ProjAndSysHelper_workFolder/checkoutDone" >/dev/null 2>&1
            rm "$__app___ProjAndSysHelper_workFolder/checkout.log" >/dev/null 2>&1

            git fetch > "$__app___ProjAndSysHelper_workFolder/checkout.log" 2>&1
            __app___ProjAndSysHelper_log_info "checking out the branch/tag: [$__app___ProjAndSysHelper___branch_or_tag]"
            git checkout -f "$__app___ProjAndSysHelper___branch_or_tag" >> "$__app___ProjAndSysHelper_workFolder/checkout.log" 2>&1
            local retCode=$?
            echo "$retCode" > "$__app___ProjAndSysHelper_workFolder/checkoutDone"
        ) &

        scheduler_waitFor anonimousFunction_31 anonimousFunction_32

     }

function anonimousFunction_24(){  
            local tskId="$1"
            if [ -f "$__app___ProjAndSysHelper_workFolder/buildTestsDone" ]; then
                scheduler_erasePeriodic "$tskId"
                
                cd "$__app___ProjAndSysHelper_projectDirectory""/tests"

                __app___ProjAndSysHelper_log_info "command 'make all (on tests project)' finished"

                local buildTestsDone=$(cat "$__app___ProjAndSysHelper_workFolder/buildTestsDone")

                if [ $buildTestsDone -ne 0 ]; then
                    __app___ProjAndSysHelper_utils_derivateError2 "Error running make all (on tests project)"
                    __app___ProjAndSysHelper_log_error "$_error"
                    
                    __app___ProjAndSysHelper_log_info "make_tests.log content (loglevel $INFO):"
                    __app___ProjAndSysHelper_log_interceptCommandStdout "$COMMAND_TRACE" "cat ./make_tests.log"
                    
                    __app___ProjAndSysHelper_telegram_sendErrorMessage "Error building the tests: $_error"
                    __app___ProjAndSysHelper_telegram_sendFile "make_tests.log"
                    
                    rm "make_tests.log"
                    $__app___ProjAndSysHelper___bldt_onDone 1 "$_error"
                    return 1
                fi
                eval "$__app___ProjAndSysHelper___bldt_onDone" 0
                return 0
            fi
            return 1
         }

function anonimousFunction_25(){ 
            if [ -f "$__app___ProjAndSysHelper_workFolder/runTestsDone" ]; then
                return 0
            fi
            return 1
         }

function anonimousFunction_26(){ 
            __app___ProjAndSysHelper_log_info "command './tests' finished"
            local testRetCode=$(cat "$__app___ProjAndSysHelper_workFolder/runTestsDone")

            __app___ProjAndSysHelper_log_info "tests.log content:"

            #go back to the folder (another parts of the code may be using the same shell)
            cd "$__app___ProjAndSysHelper_projectDirectory/tests/build"
            __app___ProjAndSysHelper_log_info "$(cat ./tests.log)"
            
            if [ $testRetCode -ne 0 ]; then
                _error="Error running the tests"
                __app___ProjAndSysHelper_log_error "$_error"
                __app___ProjAndSysHelper_log_info "tests.log content: $(cat ./tests.log)"
                __app___ProjAndSysHelper_telegram_sendErrorMessage "$_error"
                __app___ProjAndSysHelper_telegram_sendFile "tests.log"

                __app___ProjAndSysHelper___rnt_onDone 1 "$_error"
                return 1
            else
                __app___ProjAndSysHelper_log_info "Tests passed"
            fi
            rm "tests.log"
            $__app___ProjAndSysHelper___rnt_onDone 0

         }

function anonimousFunction_27(){ 
            echo "checking"
            local vssGudartControlContent=$(cat ~/vss/control)
            if [ "$vssGudartControlContent" == "stopped" ]; then
                return 0
            fi
            return 1
         }

function anonimousFunction_28(){ 
            if [ "$_error" != "" ]; then
                __app___ProjAndSysHelper_utils_derivateError2 "Error stopping vss guard"
                eval "$__app___ProjAndSysHelper_tmpOnDone \"$_error\""
            else
                eval "$__app___ProjAndSysHelper_tmpOnDone"
            fi
         }

function anonimousFunction_29(){ 
            local vssGudartControlContent=$(cat ~/vss/control)
            __app___ProjAndSysHelper_log_debug "vssGudartControlContent = $vssGudartControlContent"
            if [ "$vssGudartControlContent" == "running" ]; then
                return 0
            fi
            return 1
         }

function anonimousFunction_30(){ 
            if [ "$_error" != "" ]; then
                __app___ProjAndSysHelper_utils_derivateError2 "Error running vss guard"
                eval "$__app___ProjAndSysHelper__rvs_tmpOnDone \"$_error\""
            else
                __app___ProjAndSysHelper_log_info "Vss guard started"
                eval "$__app___ProjAndSysHelper__rvs_tmpOnDone"
            fi
         }

function anonimousFunction_31(){ 
            #check
            if [ -f "$__app___ProjAndSysHelper_workFolder/checkoutDone" ]; then
                local retCode=$(cat "$__app___ProjAndSysHelper_workFolder/checkoutDone")
                if [ $retCode -ne 0 ]; then
                    if [ -f "$__app___ProjAndSysHelper_workFolder/checkout.err" ]; then
                        _error=$(cat "$__app___ProjAndSysHelper_workFolder/checkout.err")
                    else
                        _error="Unknown error. Git output: $(cat $__app___ProjAndSysHelper_workFolder/checkout.log)"
                    fi

                    __app___ProjAndSysHelper_utils_derivateError2 "Error checking out the project"
                else
                    _error=""
                fi

                #return zero to finalize the 'waitFor' operation
                return 0
            fi
            return 1
         }

function anonimousFunction_32(){ 
            #on done

            if [ "$_error" != "" ]; then
                __app___ProjAndSysHelper_log_error "$_error"
                __app___ProjAndSysHelper_log_info "git checkout stdout/stderr: $(cat $__app___ProjAndSysHelper_workFolder/checkout.log)"
                
                rm "$__app___ProjAndSysHelper_workFolder/checkoutDone" >/dev/null 2>&1
                rm "$__app___ProjAndSysHelper_workFolder/checkout.log" >/dev/null 2>&1

                $__app___ProjAndSysHelper___chkout_onDone 1 "$_error"
                return 1
            fi

            rm "$__app___ProjAndSysHelper_workFolder/checkoutDone" >/dev/null 2>&1
            rm "$__app___ProjAndSysHelper_workFolder/checkout.log" >/dev/null 2>&1

            $__app___ProjAndSysHelper___chkout_onDone 0
            return 0
         }

