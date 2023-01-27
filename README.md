# About VSS
  Vss is a variable server. It allow you to write, change and read variables from multiple sources (apps, systems, terminal, ...).
  
  Vss is a server that provides some APIs to manage variables from multiple clients. All variables are shared between these clients, that means you can write a variable in any client and read it in any client.
  
  Vss treats variables as streams, allowing you to watch for changes and be notified when the value of the variables are changed.
  
  Vss uses a combination of memory cache and disk persistce for data. You can use the VSS as a key-value dataabse.

  The variables are write with object notation. It allow a better organization of the data and helps VSS to organize variables and notify observer and allow you to multiple variables by use of a wildcard ('*' char).

# Compiling and running
  First of wall, clone the project:

  ```bash
  git clone --recurse-submodules "http://vss repo path"
  ```

  After getting the sources using the git clone, enter in the  VSS folder and run the 'make all' command:

  ```bash
  cd VSS
  make all
  ```

  The comands above, if no error occurs, will generate a binary of the vss in the 'build' folder. The build folder will be populated with some aditional files that allow vss to run in a portable installation (more about installations modes will be discussed above).

  Now, you can enter in the build folder and run the VSS.
  ```bash
  cd build
  ./VarServer
  ```

  The VSS will startup and show some util information:

  
# Portable and integrated Modes
  
  A portable installation means vss have all it needs to work inside a unique folder. You can move this folder and rename it however you want and vss will be able to resume its work when started.

  Integrated installation means the vss will work with files inside system folders (/etc, /var, ...).

  Vss uses its configuration file to detect if it is running in a "portable mode" or in "integrated mode". If yout delete the file confs.conf, the VSS will understant that it should work in integrated mode.

  ## Portable mode
  in portable mode, the vss will use these files and folders:

    [VSS_BIN_FOLDER]/confs.conf -> configuration folder
    [VSS_BIN_FOLDER]/vss.log    -> log file
    [VSS_BIN_FOLDER]/data       -> data used and stored by THE vss

  ## Integrated mode
  in portable mode, the vss will use these files and folders:

    /etc/vss/confs.conf         -> configuration folder
    /var/log/vss.log            -> log file
    [VSS_BIN_FOLDER]/data       -> data used and stored by THE vss

  when running VSS in integrated mode, note the following:

    * The data folder is the same of the 'portable mode'
    * You must run VSS as a root (or as a user that have permissions to read from /etc and write to /var/log)
    

# Compiling and running the tests
  To run the tests, just enter in the 'test' folder and run the command 'make all':

  ```bash
  cd VSS/tests
  make all
  ```

  after compilation of tests, enter in the build folder (inside tests folder) and run the generated binary:

  ```bash
  cd build
  ./tets
  ```

# Using VSS from terminal

  ## setting and getting a variable
  You can set and get variables on terminal by use of curl command. See the examples bellow:

  ```bash
  #setting a variable
  curl -X POST -d "the value of the variable" \
  http://192.168.100.2:5023/n0/tests/testvariable

  #in this case, the variable 'n0.tests.testvariable' will be set with the value 'the value of the variable'. If the variable not exists, it will be created.
  ```

  ```bash
  #getting a variable value
  curl http://192.168.100.2:5023/n0/tests/testvariable
  #result: {"n0":{"tests":{"testvariable":{"_value":"The value of the variable"}}}}

  #this command will get the value of 'n0/tests/testvariable' in a json (the default result format of the HTTP API).

  #you can also request the data in the format of a plain text, adding the header 'accept' to the request:
  curl -H "accept: text/plain" http://192.168.100.2:5023/n0/tests/testvariable
  #result: n0.tests.testvariable=the value of the variable
  ```

# Task lists
## Main task List
    charaters to be used ✔ ✘
    [✔] Change folder 'source' name to 'sources' (look in the 'rastreio correios' project to see the makefile)
    [✔] Alias is realy needed ? Remove alias system
    [✔] Very important: Mutex variables (lock and u \nlock). 'lock' o(setAndUnlock) and 'unlock' (setVar can't change this variables when locked). An ideia: lock can return an token that can be used by an special setLockedVar to change this value (this allow just one client to change a locked variable).
    [✔] Variables started with '_' are internal fla \gs. Do not allothis names
    [✔] Variable persistency
    [✔] Configuration system (to specify)
    [✔] UDP replay to server search
    
    [✔] Create a logger library
    [✔] Create the configuration system (observable \ configuratiosystem)
    [✔] Create the Controller_VarHelper to isolate v \ars logic forController
  
    
    [✔] Convert TCP server to a repository
  
    [✔] Analyse the possibility of write var files  \in paralel (to besperformance/best latency)
    [✔] Import new Logger lib repository
    [✔] Import the TCPServer repo
    [ ] Erros while setting variable with '*' char should be returned to the API (that will be able to return the error to the client)
    [✔] When a client reconnects, the server needs to update it on all the variables it is looking at
    [✔] Add HTTP Api
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
    [ ] Shared->Libs->TcpServer
    [ ] Shared->Misc->DynamicVar
    [ ] Shared->Misc->Observable
    [✔] Shared->Misc->TaggedObject
    [ ] Services->ServerDiscovery
    [ ] Services->APIs->Http

## sugests and things to analyze    
    - Restfull and WebSocket API
    - Rename Controller to Business, Logic, core or core service (to analyze. It prevent confusing with MVC system)
    - Paralel project (app server)
    - Move all code of main.cpp to a class
    - Convert configs to a repository

# Project Progress (24/32)
    ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░ ~75%



# Further information

An overview on how VSS work with data when a 'var set' in requested
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
  |  var set notifi-   |    |<-+                  |  |
  |  cations           |<---|                     |  |
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

# things to study and other information
When i wrote this app, I reused ancient codes, that was not very well structured. So some things should be refactored.

-> Dismember VSTP service/module in the following structure (just a suggestion):
    serviceController - Orchestrate the workflow
    VSTP read and writer - recieve and write data to web sockets, puttin headers, sizes and checksums
    pack processor and generator - interprete and create packs in the VSTP format (command + data)