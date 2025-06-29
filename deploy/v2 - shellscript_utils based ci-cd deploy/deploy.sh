#!/bin/bash

ProjectName="VarStreamServer CI/CD"
GitRepo="/mnt/flashdrive64gb/projects_repos/VarStreamServer"
TlgBotKey="5796174720:AAH8ogjGOF7aChcBRvjiHe1vOJSKHOasq7c"
TlgChatId="165633302"

# initialization of program and class initialization {
    if [ "$1" != "new" ]; then 
        echo "initing SHU (scanning for classes)"
        source "../../shellscript_utils/src/program.sh" ""
        program_init "../../shellscript_utils/src" 1

        new_f "$0" __app__ "$@"
        scheduler_workLoop 1
        exit 0; 
    fi

    #inherits BasicDeploy
    inherits "SimpleDeployBase" "$this->name" ""

    this->init(){
        new "utils/utils.sh" "this->utils"
        new "telegramsend" "this->telegram" "$TlgBotKey" "$TlgChatId" "<u>$ProjectName:</u>\n\n"

        mkdir -p run
        cd run

        #calls base "constructor" (the init method)
        this->SimpleDeployBase->init "VarStreamServer" "$GitRepo" false
        this->logTerminalWriter->_logLevel=0

        # initializes the project and system helper {
            #autoinit=false; new "ProjectAndSysHelper/MockProject" "this->ProjAndSysHelper"
            autoinit=false; new "ProjectAndSysHelper/VssProject" this->ProjAndSysHelper
            
            this->ProjAndSysHelper->init "this->logManager" "this->telegram" "$GitRepo"
            newRef "$_r" this->ProjAndSysHelperPromise

            if [ "$this->ProjAndSysHelperPromise->class" != "promise" ]; then
                this->utils->derivateError2 "Project and system helper did not return a promise"
                this->log->error "$_error"
                exit 1
                return 1
            fi

            this->ProjAndSysHelperPromise->catch _(){
                _error="$1"
                this->utils->derivateError2 "Error creating project and system helper"
                this->log->error "$_error"
                exit 1
                return 1
            }()_
            
            this->ProjAndSysHelperPromise->then _(){
                this->log->info "Project and system helper created"
                this->startupGitObserver
            }()_
        #}
    }

    this->finalize(){
        finalize this->ProjAndSysHelper
        finalize this->utils

        #finalize the base class
        finalize this->SimpleDeployBase
    }
#}

# pipes {
    #overrides the 'defaultPip' method
    #default pipe (the continious integration), runs the tests when a new commit is pushed to the repository. Receives a gitobserver::StructBranchCommits object
    this->defaultPipe(){ local _StructCommitInfoObj_="$1"
        new promise "this->_defpipe_retProm"

        deserializeObject "this->_sampleObj" "$_StructCommitInfoObj_"
        
        this->log->info "checking out to branch [$this->_sampleObj->branchName]"
        echo '-------------------------------------------------------------------------------------------------------------------'
        this->ProjAndSysHelper->checkout "$this->_sampleObj->branchName" _(){
            echo "Ok, checkou rolou de boas"
            if [ $1 -ne 0 ]; then
                this->utils->derivateError2 "Error checking out the commit (default pipe)"
                this->log->error "$_error"
                this->telegram->sendErrorMessage "$_error"
                this->_defpipe_retProm->reject 1 "$_error"
                return 1
            fi

            #telegram resume #{
                local telegramMsg="New deploy pipeline started for branch $this->_sampleObj->branchName:"
                telegramMsg+="\n    - Last commit: $this->_sampleObj->lastCommit"
                this->_getAuthorsFromCommitInfoObj "this->_sampleObj"
                telegramMsg+="\n    - Authors: $_r"
                telegramMsg+="n\nRunning CD pipe for the commit"

                this->log->info "$telegramMsg"

                this->telegram->sendInfoMessage "$telegramMsg"

                finalize "this->_sampleObj"
                
                local pipelineText="Pipeline:\n"
                local pipelineText+="    1) Build tests\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    2) Run tests\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    3) Build main project\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    4) Upload artifacts\n"
                
                this->telegram->sendTextMessage "$pipelineText"

            #}
        
            this->_pipesBase
            newRef $_r theProm

            theProm->then _(){
                this->log->info "CD Pipeline finished with success"
                this->telegram->sendTextMessage "CI Pipeline finished with success"
                finalizeRef "theProm" true
                this->_defpipe_retProm->resolve
            }()_

            theProm->catch _(){ local code=$1; local error="$2"
                finalizeRef "theProm" true
                this->utils->derivateError2 "Error running common pipe (from default pipe)"

                this->_defpipe_retProm->reject "$code" "$_error"
            }()_
        }()_

        _r="this->_defpipe_retProm"
        return 0
    }

    #overrides the 'releasePipe' method
    #when a new tag is pushed to the repository, this pipe is executed, it runs the tests and release the new version
    this->releasePipe(){ local tag=$1; local commit=$2;
        new promise "this->_relpipe_retProm"        

        this->log->info "checking out to tag [$tag]"
        this->ProjAndSysHelper->checkout "$tag" _(){
            if [ $1 -ne 0 ]; then
                this->utils->derivateError2 "Error checking out the commit (release pipe)"
                this->log->error "$_error"
                this->telegram->sendErrorMessage "$_error"

                this->_relpipe_retProm->reject 1 "$_error"
                return 1
            fi

            #telegram resume #{
                local info="New tag ($tag) pushed to the repository. Running CD pipe for the commit: $commit"
                this->log->info "$info"
                this->telegram->sendInfoMessage "$info"

                local pipelineText="Pipeline:\n"
                local pipelineText+="    1) Build tests\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    2) Run tests\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    3) Build main project\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    4) Upload artifacts\n"
                local pipelineText+="            ↓\n"
                local pipelineText+="    5) Deploy to production\n"

                this->telegram->sendTextMessage "$pipelineText"
            #}

            # this->defaultPipe "$commit" "$tag"

            this->_pipesBase "$tag"; newRef $_r pipesBasePromise

            pipesBasePromise->catch _(){ local code=$1; local error="$2"
                finalizeRef "pipesBasePromise" true
                this->utils->derivateError2 "Error running common pipe (from release pipe)"

                this->_relpipe_retProm->reject "$code" "$_error"
            }()_

            pipesBasePromise->then _(){
                this->log->info "running stage '5) Deploy to production'"
                this->telegram->sendTextMessage "running stage '5) Deploy to production'"

                finalizeRef "pipesBasePromise" true
                
            
                this->ProjAndSysHelper->deployToProduction "$tag" _(){
                    local errorCode="$1"
                    echo "errorCode: $errorCode"
                    local errorMessage="$2"
                    echo "errorMessage: $errorMessage"

                    if [ $errorCode -ne 0 ]; then
                        _error="$errorMessage"
                        this->utils->derivateError2 "Error deploying to production"
                        this->telegram->sendErrorMessage "$_error"

                        this->_relpipe_retProm->reject "$errorCode" "$_error"
                        return 1
                    else
                        this->log->info "Deploy Pipeline finished with success"
                        this->telegram->sendTextMessage "Deploy Pipeline finished with success"
                        this->_relpipe_retProm->resolve
                    fi

                    return 0
                }()_
            }()_
        }()_

        _r="this->_relpipe_retProm"
    }

    #this method contains the code to make things (build, run test, etc) that are need by both defaultPipe and releasePipe
    this->_pipesBase(){ this->_tmp_customArtifactInfo_="$1"
        new promise.sh > /dev/null
        newRef "$_r" "this->_pb_retProm"
    
        this->__tmp_stage1(){ this->__stage1_onDoneCallback="$1"
        this->log->info "running stage '1) Build tests'"
            this->telegram->sendTextMessage "running stage '1) Build tests'"

            this->ProjAndSysHelper->buildTests _(){
                if [ $1 -ne 0 ]; then
                    this->utils->derivateError2 "Error building the tests"
                    this->telegram->sendErrorMessage "$_error"
 
                    $this->__stage1_onDoneCallback 1 "$_error"
                    return 1
                fi
                $this->__stage1_onDoneCallback 0
                return 0
            }()_
        }
        
        this->__tmp_stage2(){ this->__stage2_onDoneCallback="$1"
        this->log->info "running stage '2) Run tests'"
            this->telegram->sendTextMessage "running stage '2) Run tests'"
            this->ProjAndSysHelper->runTests _(){
                if [ $1 -ne 0 ]; then
                    this->utils->derivateError2 "Error running the tests"
                    this->telegram->sendErrorMessage "$_error"
                    $this->__stage2_onDoneCallback 1 "$_error"
                    return 1
                fi
                $this->__stage2_onDoneCallback 0
                return 0
            }()_
        }

        this->__tmp_stage3(){ this->__stage3_onDoneCallback="$1"
            if [ -z "$this->_tmp_customArtifactInfo_" ]; then
                this->_tmp_customArtifactInfo_=$(git rev-parse --short $commit)
            fi

            this->log->info "running stage '3) Build main project'"
            this->telegram->sendTextMessage "running stage '3) Build main project'"

            this->ProjAndSysHelper->buildArtifacts "$this->_tmp_customArtifactInfo_" _(){
                if [ $1 -ne 0 ]; then
                    this->utils->derivateError2 "Error building artifacts (build main project)"
                    this->telegram->sendErrorMessage "$_error"
                    $this->__stage3_onDoneCallback 1 "$_error"
                    return 1
                fi
                $this->__stage3_onDoneCallback 0
                return 0
            }()_
        }

        this->__tmp_stage4(){ this->__stage4_onDoneCallback="$1"
            if [ -z "$this->_tmp_customArtifactInfo_" ]; then
                this->_tmp_customArtifactInfo_=$(git rev-parse --short $commit)
            fi

            this->log->info "running stage '4) Upload artifacts'"
            this->telegram->sendTextMessage "running stage '4) Upload artifacts'"
            
            this->ProjAndSysHelper->uploadArtifacts "$this->_tmp_customArtifactInfo_" _(){
                if [ $1 -ne 0 ]; then
                    this->utils->derivateError2 "Error building artifacts (build main project)"
                    this->telegram->sendErrorMessage "$_error"
                    $this->__stage4_onDoneCallback 1 "$_error"
                    return 1
                fi
                $this->__stage4_onDoneCallback 0
                return 0
            }()_
        }

        this->__currStage=0
        this->runNextStage(){ local resultCode=$1
            if [ -z "$resultCode" ]; then
                resultCode=0
            fi
            

            if [ $resultCode -ne 0 ]; then
                this->log->error "Error running stage $this->__currStage: $_error"
                this->_pb_retProm->reject "$resultCode" "$_error"
                return
            fi

            this->__currStage=$((this->__currStage+1))
            scheduler->run _(){
                if [ $this->__currStage -eq 1 ]; then
                    this->__tmp_stage1 "this->runNextStage"
                elif [ $this->__currStage -eq 2 ]; then
                    this->__tmp_stage2 "this->runNextStage"
                elif [ $this->__currStage -eq 3 ]; then
                    this->__tmp_stage3 "this->runNextStage"
                elif [ $this->__currStage -eq 4 ]; then
                    this->__tmp_stage4 "this->runNextStage"
                elif [ $this->__currStage -eq 5 ]; then
                    this->_pb_retProm->resolve
                fi
                
            }()_
        }

        scheduler->run "__f(){
            this->runNextStage 0
        }; __f"

        _r="$this->_pb_retProm"
        return 0
    }
#}

this->_getAuthorsFromCommitInfoObj(){
    newRef "$1" commitInfo

    local authors=""
    local count=$commitInfo->commitCount

    for (( i=0; i<$count; i++ )); do
        if [ "$authors" != "" ]; then
            authors+=", "
        fi

        eval "local author=\$commitInfo->commit_$i-->author"
        authors+="$author"
    done

    _r="$authors"

    finalizeRef "info"
    return 0
}