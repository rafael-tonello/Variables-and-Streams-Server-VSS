# about the files

## SimpleDeployBase.sh 
Contains a base for a deploy system. It contains an internal instance of 'gitobserver.sh' and handles some events that comes from it. SimpleDeployBase uses these events to populate and consume an internal queue and call two methods:
    defaultPipe -> when a branch receives one or more commits; 
    and releasePipe -> when a new tag is defined in the main project repository

SimpleDeployBase.sh must be inherited and its 'init' mehtod should be called from the child class.

## deploy.sh
Deploy.sh basically inherits and inits the SimpleDeployBase class and overrides its 'defaultPipe' and 'releasePipe' methods.
Deploy.sh make use of classes inside ProjectsAndSysHelper classes to interract with project and the operational system.

## ProjecAndSysHelper/IProjectAnsSys.sh
Is an interface, with methods that must be implemented by child classes. Deploy.sh class will call these methods to interact with the project and operational system.

## ProjecAndSysHelper/VssProject.sh
Contains code for build tests, run tests, build main project, deploy main project artifacts and deploy to production the Vss project

## ProjecAndSysHelper/MockProject.sh
Is a mock that call callback methods and serves to help Deploy.sh development



# TODO
    [x] simpleDeployBase.sh should use promises instead callbacks
    [ ] Create new functino in IProjectAndSys to prepare for new pipeine. This allow it to recreate docker images, etc.
    