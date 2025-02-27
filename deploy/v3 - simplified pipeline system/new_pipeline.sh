#!/bin/bash

# project configuration and specific functions
    settings_repositoryUrl="/mnt/flashdrive64gb/projects_repos/VarStreamServer"
    settings_projectName="VssCICDDev"

    settings_workingFolder="$(realpath $(dirname $0)/run2)"
    settings_workingFolderRealPath=""

    #this function will be called when the project is ready to deploy to production. 
    #pwd is the project root folder
    #$1 contains the artifact name
    custom.deployToProduction(){ local artifactName="$1"; local artifactFile="$2"
        #stop vss guard {
            echo "stop" > ~/vss/control
            local initialTime=$(date +%s)
            while true; do
                local status=$(cat ~/vss/control)
                if [ "$status" == "stopped" ]; then
                    break
                fi

                local currentTime=$(date +%s)
                local elapsedTime=$((currentTime - initialTime))
                if [ $elapsedTime -gt 60 ]; then
                    _error="Could not stop the server. Last vss control status was '$status'"
                    return 1
                fi

                sleep 1
            done
        #}

        #extract artifact (tar.gz) to $settings_workingFolder/tmp {
            tar -xzf "$artifactFile" -C "$settings_workingFolder/tmp/" > $settings_workingFolder/tar.log 2>&1
            if [ -$? -ne 0 ]; then
                _error="Could not extract artifact"
                custom.sendUserFile "$settings_workingFolder/tar.log"
                return 1
            fi
        #}

        #replace vss binaries and files with extracted ones {
            #replace binaries
            app.coutLogInfo "copying $settings_workingFolder/tmp/$artifactName/* to ~/vss/bin"
            cp -r "$settings_workingFolder/tmp/$artifactName"/* ~/vss/bin
        #}

        #start vss guard {
            app.coutLogInfo "Starting vss guard"
            echo "run" > ~/vss/control
            local initialTime=$(date +%s)
            while true; do
                local status=$(cat ~/vss/control)
                if [ "$status" == "running" ]; then
                    break
                fi

                local currentTime=$(date +%s)
                local elapsedTime=$((currentTime - initialTime))
                if [ $elapsedTime -gt 60 ]; then
                    _error="Could not start the server. Last vss control status was '$status'"
                    return 1
                fi
                sleep 1
            done
        #}

    }

    #this function is called when a message needs to be sent to the user
    custom.sendUserNotification(){
        #echo -e "$1"
        /home/orangepi/scripts/sendToTelegram.sh "sendHtml" "<u>$settings_projectName CI/CD:</u> \n🔵 $1" > /dev/null 2>&1
    }

    #this function is called when a error message needs to be sent to the user
    custom.sendUserNotification.error(){
        #echo -e "$1" > /dev/stderr
        /home/orangepi/scripts/sendToTelegram.sh "<u>$settings_projectName CI/CD:</u> \n🔴 $1" > /dev/null 2>&1
    }

    custom.sendUserNotification.warning(){
        #echo -e "$1" > /dev/stderr
        /home/orangepi/scripts/sendToTelegram.sh "<u>$settings_projectName CI/CD:</u> \n🔶 $1" > /dev/null 2>&1
    }

    #this function is called when a file needs to be sent to the user
    custom.sendUserFile(){
        #echo -e "File: $1" > /dev/stderr
        /home/orangepi/scripts/sendToTelegram.sh "sendFile" "$1" > /dev/null 2>&1
    }
#}

# internal / generic operation functions
    app.main(){
        app.coutLogInfo "Checking for new changes in the repository\n\n"

        app.cloneRepository
        if [ $? -ne 0 ]; then
            _error="Error while cloning repository: $_error"
            app.error "$_error"
            return 1
        fi

        app.coutLogInfo "getting branches"
        app.getBranches #response comes in the _r variable (an array)
        if [ $? -ne 0 ]; then
            _error="Error while getting branches: $_error"
            app.error "$_error"
            return 1
        fi

        #copy _r array to 'branches' array
        local branches=("${_r[@]}")
        #ecroll through the branches
        for branch in "${branches[@]}"; do
            app.coutLogInfo "Checking branch $branch for new commints (since the last execution)"
            app.getNewCommit "$branch" 
            if [ -z "$_error" ]; then
                if [ -z "$_r" ]; then
                    _error="Bug: An error ocurred. A commit should be returned for branch '$branch', but nothing was found"
                    app.error "$_error"
                    app.warning "Besides the error, the pipeline will continue for the next branch"
                    continue
                fi

                app.coutLogInfo "running default pipeline for branch $branch"
                app.defaultPipeline "$branch"
                if [ $? -ne 0 ]; then
                    _error="Error while running default pipeline for branch '$branch': $_error"
                    app.error "$_error"
                    app.warning "Besides the error, the pipeline will continue for the next branch"
                fi
            fi
        done

        app.coutLogInfo "checking for new tags"
        app.getNewTag
        if [ -z "$_error" ]; then
            if [ -z "$_r" ]; then
                _error="Bug: An error ocurred. A tag should be returned, but nothing was found"
                app.error "$_error"
                app.warning "Besides the error, the pipeline will continue for the next branch"
                continue
            fi

            app.deployPipeline "$_r"
            if [ $? -ne 0 ]; then
                _error="Error while running deploy pipeline for tag '$_r': $_error"
                app.error "$_error"
            fi
        fi

        #a file named request.default can be create to request execution of a default pipeline. 
        #Each line should have the following format: branchName commitHash
        if [ -f "request.default" ]; then
            while read request; do
                app.defaultPipeline "$request"
                if [ $? -ne 0 ]; then
                    _error="Error while running default pipeline for request '$request': $_error"
                    app.error "$_error"
                    app.warning "Besides the error, the pipeline will continue for the next request"

                fi
            done < request.default

            rm request.default
        fi

        #a file named request.deploy can be create to request execution of a deploy pipeline. 
        #Each line should have the following format: tagName
        if [ -f "request.deploy" ]; then
            while read request; do
                app.deployPipeline "$request"
                if [ $? -ne 0 ]; then
                    _error="Error while running deploy pipeline for request '$request': $_error"
                    app.error "$_error"
                    app.warning "Besides the error, the pipeline will continue for the next request"
                fi
            done < request.deploy

            rm request.deploy
        fi

    }

    app.cloneRepository(){
        mkdir -p "$settings_workingFolder"

        app_state_projectCodeWithNoSubmodules="$settings_workingFolder/projectCodeWithNoSubmodules"
        if [ ! -d "$app_state_projectCodeWithNoSubmodules" ]; then
            app.coutLogInfo "Cloning repository with submodules to check branches and tags"

            git clone $settings_repositoryUrl "$app_state_projectCodeWithNoSubmodules" > $settings_workingFolder/clone.log 2>&1
            app.coutFile "$settings_workingFolder/clone.log" "Git clone (with submodules) output:"
            if [ -$? -ne 0 ]; then
                _error="Could not clone repository (with no submodules) for checking of branches and tags"
                _error="$_error"
                custom.sendUserFile "$settings_workingFolder/clone.log"
                return 1
            fi

        fi

        _error=""
        return 0
    }

    app.getBranches(){
        cd "$app_state_projectCodeWithNoSubmodules"
        local branches=$(git branch -a | grep remotes | grep -v HEAD | cut -d '/' -f3-)
        _r=($branches)
    }

    #_r receives the last commit if new commits are found. _error receives the error 
    #message if any error occurs or nothing is found
    app.getNewCommit(){ local branchName="$1"; local _save_last_commit_if_found_def_true_="$2"
        #params validation {
            if [ -z "$branchName" ]; then
                _error="getNewCommit -> branchName is required"
                return 1
            fi

            if [ -z "$_save_last_commit_if_found_def_true_" ]; then
                _save_last_commit_if_found_def_true_=true
            fi
        #}

        cd "$app_state_projectCodeWithNoSubmodules"
        git fetch origin $branchName > $settings_workingFolder/fetchorigin.log 2>&1
        app.coutFile "$settings_workingFolder/fetchorigin.log" "Git fetch origin output:"
        if [ -$? -ne 0 ]; then
            _error="Could not fetch branch '$branchName'"
            _error="$_error"
            custom.sendUserFile "$settings_workingFolder/fetchorigin.log"
            return 1
        fi

        local lastCommit=$(git log origin/$branchName -1 --pretty=format:"%H")
        if [ -$? -ne 0 ]; then
            _error="Could not get last commit for branch '$branchName'"
            _error="$_error"
            return 1
        fi

        app.storage.get "$branchName.lastCommit" ""; local lastSavedCommit="$_r"
        _r=""
        if [ "$lastCommit" != "$lastSavedCommit" ]; then
            if $_save_last_commit_if_found_def_true_; then
                app.storage.set "$branchName.lastCommit" "$lastCommit"
            fi

            _r="$lastCommit"
            _error=""
            return 0
        else
            _error="No new commits found for branch '$branchName'"
            return 1
        fi
    }

    #_r receives the last tag if new tags are found. _error receives the error
    #message if any error occurs or nothing is found
    app.getNewTag(){ local _save_last_tag_if_found_def_true_="$1"
        if [ -z "$_save_last_tag_if_found_def_true_" ]; then
            _save_last_tag_if_found_def_true_=true
        fi

        cd "$app_state_projectCodeWithNoSubmodules"
        git fetch --tags > $settings_workingFolder/fetchtags.log 2>&1
        app.coutFile "$settings_workingFolder/fetchtags.log" "Git fetch tags output:"
        if [ -$? -ne 0 ]; then
            _error="Could not fetch tags"
            _error="$_error"
            custom.sendUserFile "$settings_workingFolder/fetchtags.log"
            return 1
        fi

        local lastTag=$(git describe --tags `git rev-list --tags --max-count=1`)
        if [ -$? -ne 0 ]; then
            _error="Could not get last tag"
            _error="$_error"
            return 1
        fi

        app.storage.get "lastTag" ""; local lastSavedTag="$_r"
        _r=""
        if [ "$lastTag" != "$lastSavedTag" ]; then
            if $_save_last_tag_if_found_def_true_; then
                app.storage.set "lastTag" "$lastTag"
            fi
            
            _r="$lastTag"
            _error=""
            return 0
        else
            _error="No new tags found"
            return 1
        fi
    }

    #_r receives the artifact name
    app.defaultPipeline(){ local branch="$1"; local _display_pipeline_="$2"; _delete_folder_after_done_="$3"
        #params validation {
            if [ -z "$branch" ]; then
                _error="defaultPipeline -> Branch is required"
                return 1
            fi
            
            if [ -z "$_display_pipeline_" ]; then
                _display_pipeline_=true
            fi

            if [ -z "$_delete_folder_after_done_" ]; then
                _delete_folder_after_done_=true
            fi
        #}

        #clone project if not cloned (when running direct from command line, without the main function){
            app.cloneRepository
            if [ $? -ne 0 ]; then
                _error="Error while cloning repository: $_error"
                app.error "$_error"
                return 1
            fi
        #}

        if $_display_pipeline_; then
            local notificationContent="Default pipeline is running for branch $branch\n\n"
            notificationContent+="Pipeline: \n"
            notificationContent+="  1. Checkout project\n"
            notificationContent+="  2. Build tests\n"
            notificationContent+="  3. Run tests\n"
            notificationContent+="  4. Build main project\n"
            notificationContent+="  5. Upload artifacts\n"
            app.info "$notificationContent"
        fi

        #checkout {
            app.info "Checking out project ..."
            app_state_tmpFolder=$settings_workingFolder/$(mktemp -d)

            git clone --recursive $settings_repositoryUrl $app_state_tmpFolder -b "$branch" > $settings_workingFolder/clone.log 2>&1
            app.coutFile "$settings_workingFolder/clone.log" "Git clone (complete, with submodules) output:"
            if [ -$? -ne 0 ]; then
                _error="1. Checkout project -> Could not clone repository"
                app.error "$_error"
                custom.sendUserFile "$settings_workingFolder/clone.log"
                return 1
            fi


            # cd $app_state_tmpFolder
            # git checkout "$branch" > $settings_workingFolder/checkout.log 2>&1
            # app.coutFile "$settings_workingFolder/checkout.log" "Git checkout output:"
            # if [ -$? -ne 0 ]; then
                # _error="1. Checkout project -> Could not checkout branch"
                # app.error "$_error"
                # custom.sendUserFile "$settings_workingFolder/checkout.log"
                # return 1
            # fi


            app.info "1. Checkout project -> Sucess"
        #}

        #build tests {
            app.info "Building tests ..."
            cd $app_state_tmpFolder/tests
            nice -n 19 make all -j 4 > make.log 2>&1
            if [ -$? -ne 0 ]; then
                #only print make.log file in errors cases (make.log could have too much content)
                app.coutFile "make.log" "make.log file content:"

                _error="2. Build tests -> Could not build tests"
                app.error "$_error"
                custom.sendUserFile "make.log"
                return 1
            fi
            app.info "2. Build tests -> Sucess"
        #}

        #run tests {
            app.info "Running tests ..."
            cd build
            ./tests > tests.log 2>&1
            app.coutFile "tests.log" "tests.log file content:"
            if [ -$? -ne 0 ]; then
                _error="3. Run tests -> Could not run tests"
                app.error "$_error"
                custom.sendUserFile "tests.log"
                return 1
            fi
            app.info "3. Run tests -> Sucess"
        #}

        #build main project {
            cd $app_state_tmpFolder
            app.info "Building main project ..."
            nice -n 19 make all -j 4 > make.log 2>&1
            if [ -$? -ne 0 ]; then
                #only print make.log file in errors cases (make.log could have too much content)
                app.coutFile "make.log" "make.log file content:"
                
                _error="4. Build main project -> Could not build main project"
                app.error "$_error"
                custom.sendUserFile "make.log"
                return 1
            fi
            app.info "4. Build main project -> Sucess"
        #}

        #upload artifacts {
            cd $app_state_tmpFolder
            local currentArchitechture=$(uname -m)
            app.info "Creating and uploading artifacts ..."
            local artifactName="$settings_projectName-$branch-$currentArchitechture"
            local outputFile="$artifactName.tar.gz"
            app.coutLogInfo "copying build folder to $artifactName"
            cp -r ./build $artifactName
            tar -czf $outputFile $artifactName > tar.log 2>&1
            if [ -$? -ne 0 ]; then
                _error="5. Upload artifacts -> Could not create artifacts"
                app.error "$_error"
                custom.sendUserFile "tar.log"
                return 1
            fi

            custom.sendUserFile "$outputFile"
            if [ -$? -ne 0 ]; then
                _error="5. Upload artifacts -> Could not upload artifacts: $_error"
                app.error "$_error"
                custom.sendUserFile "tar.log"
                return 1
            fi

            app.info "5. Upload artifacts -> Sucess"
        #}

        _r_tmpFolder="$app_state_tmpFolder"
        if $_delete_folder_after_done_; then
            rm -rf $app_state_tmpFolder
        fi

        _r="$artifactName"
        _r_artifactFile="$outputFile"

        _error=""
        return 0
    }

    app.deployPipeline(){ local tag="$1";
        #params validation {
            if [ -z "$tag" ]; then
                _error="deployPipeline -> tag is required"
                return 1
            fi
        #}

        #clone project if not cloned (when running direct from command line, without the main function){
            app.cloneRepository
            if [ $? -ne 0 ]; then
                _error="Error while cloning repository: $_error"
                app.error "$_error"
                return 1
            fi
        #}

        local notificationContent="Deploy pipeline is running for tag $tag\n\n"
        notificationContent+="Pipeline: \n"
        notificationContent+="  1. Checkout project\n"
        notificationContent+="  2. Build tests\n"
        notificationContent+="  3. Run tests\n"
        notificationContent+="  4. Build main project\n"
        notificationContent+="  5. Upload artifacts\n"
        notificationContent+="  6. Deploy to production\n"

        app.info "$notificationContent"

        app.defaultPipeline "$tag" false false
        if [ $? -ne 0 ]; then
            _error="Could not run default pipeline for tag '$tag': $_error"
            app.error="$_error"
            return 1
        fi

        #deploy to production {
            custom.deployToProduction "$_r" "$_r_artifactFile"
            if [ $? -ne 0 ]; then
                _error="Could not run default pipeline for tag '$tag': $_error"
                app.error "$_error"
                rm -rf "$_r_tmpFolder"
                return 1
            fi

            rm -rf "$_r_tmpFolder"

            app.info "6. Deploy to production -> Sucess"
        #}
        _error=""
        return 0

    }

    app.storage.set(){ local key="$1"; local value="$2";
        app.utils.getValidKey "$key"; key="$_r"
        if [ ! -d "$settings_workingFolder/db" ]; then
            mkdir -p "$settings_workingFolder/db"
        fi

        echo "$value" > "$settings_workingFolder/db/$key"
    }

    app.storage.get(){ local key="$1"; local _default_="$2";
        app.utils.getValidKey "$key"; key="$_r"
        if [ ! -f "$settings_workingFolder/db/$key" ]; then
            _r="$_default_"
        else
            _r=$(cat "$settings_workingFolder/db/$key")
        fi
    }

    app.utils.getValidKey(){ local key="$1";
        _r=$(echo "$key" | tr -d '[:space:]' | tr '[:upper:]' '[:lower:]')
        #remove special characters
        _r=$(echo "$_r" | sed 's/[^a-zA-Z0-9]//g')
    }

    #log errors and alsl send it to the user
    app.error(){
        #print red text
        printf "\033[0;31m" > /dev/stderr
        app.coutLogError "$1"
        printf "\033[0m\n" > /dev/stderr

        custom.sendUserNotification.error "$1"
    }

    #log info and also send it to the user
    app.info(){
        app.coutLogInfo "$1"
        custom.sendUserNotification "$1"
    }

    #log warning and also send it to the user
    app.warning(){
        app.coutLogWarning "$1"
        custom.sendUserNotification.warning "$1"
    }

    app.help(){
        echo "Usage: $0 [option/command]"
        echo "Options/commands:"
        echo "    help, --help, -h         -Displays this help text"
        echo ""
        echo "    defaultPipeline <branch> -Run the default pipeline for the given branch and"
        echo "                              commit"
        echo ""
        echo "    deployPipeline <tag>     -Run the deploy pipeline for the given tag. You can"
        echo "                              use a branch name instead of a tag, but it is not"
        echo "                              recommended"
        echo ""
        echo "    main                     -Normal execution. Will check if there are new"
        echo "                              commits or tags since the last execution and run"
        echo "                              the pipelines accordingly (runs 'defaultPipeline'"
        echo "                              for new commits in the projects branches and runs"
        echo "                              'deployPipeline' command for new tags)"
        echo ""
        echo "    monitor [interval]       -Will call the main command every [interval]"
        echo "                              seconds. If no interval is given, the value 60 will"
        echo "                              be used"
        echo ""
        echo ""
        echo "Note: If no command is given, the main command will be executed"

        echo ""
        echo "Examples:"
        echo "    $0"
        echo "          -Looks for new commits and tags, run the respective pipelines and exit."
        echo ""
        echo "    $0 defaultPipeline develop"
        echo "          -Runs the default pipeline for the branch 'develop' and exit."
        echo ""
        echo "Additional informations:"
        echo "      default pipeline: Checkout project, build tests, run tests, build main project"
        echo "      and upload artifacts"
        echo ""
        echo "      deploy pipeline: default pipeline + deploy to production"


    }

    #just print info logs to the stdout
    app.coutLogInfo(){
        echo -e "[$(date)][INFO] $1"
    }

    #just print warning logs to the stdout
    app.coutLogWarning(){
        echo -e "[$(date)][WARNING] $1"
    }

    #just print error logs to the stderr
    app.coutLogError(){
        echo -e "[$(date)][ERROR] $1" > /dev/stderr
    }

    #print the content of a file to the stdout (nested to a information log line)
    app.coutFile(){ local fName="$1"; local title="$2"
        if [ ! -z "$title" ]; then
            app.coutLogInfo "$title"
        fi

        while read line; do
            echo "        $line"
        done < "$fName"
    }

    app.monitor(){ local _interval_="$1"; shift;
        if [ -z "$_interval_" ]; then _interval_=60; fi
        app.coutLogInfo "Running in monitor mode. Repository will be checked every $_interval_ seconds"

        while true; do
            app.main "$@"
            sleep $_interval_
        done;
    }

    if [ -z "$1" ]; then
        app.main
        exit $?
    elif [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
            app.help
            exit 0
    else
        func="$1"
        shift
        app.$func "$@"
        exit $?
    fi
#}
