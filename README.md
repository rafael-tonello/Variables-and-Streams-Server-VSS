# About VSS
  "Vss is a variable server. It means you can write and read variables from it". This is simplest way to describe the VSS. Vss is a server that provides some APIs to manage variables from multiple clients. 
  All variables are shared between clients, that means you can write a variable in any client and read it in any client.
  In addition to being read and written, variables are also streams, meaning you can watch them and be notified when their values ​​change.

  These variables can be write with object notation. For instance, you can set a variable named "parent.child.child2". It allow a beter organization and allow some aditional features, like read a lot of varibales usina the * character and observe a lot of variables (also by use of * character).

# General information
  I wrote this code in the Visual Studio Code, so , some functions will be documented using a notation
  that can be used by this IDE.


  I Used to use the 'tests' project during the development to make unity and integrations tests. Eventually I compile the main project.
  Whena new version is done, i run the deploy scripts (inside 'deploy' folder).

# compiling and installing 
  The simplest way to compile the VSS (Var Stream Server) is just run 'make' command in the project root folder. The project will be compiled and, if no error occurs, a portable version of the VSS will appear in the 'build' folder.
  
  If you want to a more integrated installation than the 'portable version', you can 'run make install' after the 'make' command, and the vss will be installed in you server. Doing this the program will be installed in the /usr/bin folder. Configuration files will be moved to /etc/VSS folder and the data folder will be /var/VSS/data. Also a systemd daemon will be created with the name VSS.service and, then, this service will be enabled and started.

  Inside the folder 'delploy', you will find more complete installation scripts, which before compiling and installing, will run unity tests.
  
  OBS.: If you want to run unity testes, run the 'make' command inside the './tests' folder. After the build, enter in the folder './tests/build' and run the 'tests' binary


# things to do..
  When i wrote this app, I reused ancient codes, that was not very well structured. So some things should be refactored.

  -> Dismember VSTP service/module in the following structure (just a suggestion):
      serviceController - Orchestrate the workflow
      VSTP read and writer - recieve and write data to web sockets, puttin headers, sizes and checksums
      pack processor and generator - interprete and create packs in the VSTP format (command + data)


# Task lists
## Main task List
    charaters to be used ✔ ✘
    [✔] Change folder 'source' name to 'sources' (look in the 'rastreio correios' project to see the makefile)
    [✔] Alias is realy needed ? Remove alias system
    [✔] Very important: Mutex variables (lock and u \nlock). 'lock' o(setAndUnlock) and 'unlock' (setVar can't change this variables when locked). An ideia: lock can return an token that can be used by an special setLockedVar to change this value (this allow just one client to change a locked variable).
    [✔] Variables started with '_' are internal fla \gs. Do not allothis names
    [✔] Variable persistency
    [✔] Configuration system (to specify)
    [ ] UDP replay to server search
    [ ] Restfull and WebSocket API
    [ ] Rename Controller to Business, Logic, core or core service (to analyze. It prevent confusing with MVC system)
    [✔] Create a logger library
    [✔] Create the configuration system (observable \ configuratiosystem)
    [ ] Paralel project (app server)
    [✔] Create the Controller_VarHelper to isolate v \ars logic forController
    [ ] Move all code of main.cpp to a class
  
    [ ] Convert configs to a repository
    [✔] Convert TCP server to a repository
  
    [ ] Create mirror service (to mirror current ser \ver in a anotheserver - a parent server). This mirror server should prevent 'back notification' from the remote server.
    [✔] Analyse the possibility of write var files  \in paralel (to besperformance/best latency)
    [✔] Import new Logger lib repository
    [✔] Import the TCPServer repo
    [ ] Erros while setting variable with '*' char should be returned to the API (that will be able to return the error to the client)
## Tests
    [ ] Controller
    [✔] ControllerClientHelper
    [✔] ControllerVarHelper
    [✔] Services->APis->VSTP
    [✔] Services->APis->VSTP->socketInfo
    [ ] Services->Storage->VarSystemLib
    [✔] Shared->Libs->Confs
    [✔] Shared->Libs->Confs->SimpleConfFileProvider
    [✔] Shared->Libs->DependencyInjectionManager
    [ ] Shared->Libs->logger
    [ ] Shared->Libs->logger->LoggerConsoleWriter
    [ ] Shared->Libs->logger->LoggerFileWriter
    [ ] Shared->Libs->logger->LoggerLambdaWriter
    [ ] Shared->Libs->TcpServer
    [ ] Shared->Misc->DynamicVar
    [ ] Shared->Misc->Observable
    [✔] Shared->Misc->TaggedObject

# Project Progress (20/38)
    ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░ ~50%

```
+---------+           +------------ server -----------+
| client  |           |  +-----+            +------+  |
+---------+           |  | RAM |            | disk |  |
  |                   |  +-----+            +------+  |
  |                   +-----|---------------------|---+
  |                    |    |                     |
  |                    |    |                     |
  |   var set          |    |                     |
  |-----'------------->|    |                     |
  |                    |--->|--+  paralel process |
  |                    |-------|--------'-------->|
  |                    |    |  |                  |--+
  |                    |    |<-+                  |  |
  |  var set response  |<---|                     |  |
  |<---------'---------|    |                     |  |
  |                    |    |                     |  | Write to disk
  |                    |    |                     |  | process may
  |                    |    |                     |  | take a long
  |                    |    |                     |  | time
  |                    |    |                     |  |
  |                    |    |                     |  |
  |                    |    |                     |<-+
  |                    |    |                     |
```
