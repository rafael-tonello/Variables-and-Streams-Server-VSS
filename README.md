# General information
    I wrote this code in the Visual Studio Code, so , some functions will be documented using a notation
    that can be used by this IDE.


    I Used to use the 'tests' project during the development to make unity and integrations tests. Eventually I compile the main project.
    Whena new version is done, i run the deploy scripts (inside 'deploy' folder).

# things to do..
    When i wrote this app, I reused ancient codes, that was not very well structured. So some things should be refactored.

    -> Dismember PHOMAU service/module in the following structure (just a suggestion):
        serviceController - Orchestrate the workflow
        phomau read and writer - recieve and write data to web sockets, puttin headers, sizes and checksums
        pack processor and generator - interprete and create packs in the PHOMAU format (command + data)


# Main task List
charaters to be used ✔ ✘
[✔] Alias is realy needed ? Remove alias system
[ ] Very important: Mutex variables (lock and unlock). 'lock' or (setAndUnlock) and 'unlock' (setVar can't change this variables when locked). An ideia: lock can return an token that can be used by an special setLockedVar to change this value (this allow just one client to change a locked variable).
[✔] Variables started with '_' are internal flags. Do not allow this names
[✔] Variable persistency
[✔] Configuration system (to specify)
[ ] UDP replay to server search
[ ] Restfull and ws API
[ ] Rename Controller to Business or Logic (to analyze. Is to prevent confuse with MVC system)
[✔] Create a logger library
[✔] Create the configuration system (observable configuration system)
[ ] Paralel project (app server)

[ ] Convert configs to a repository
[✔] Convert TCP server to a repository

[ ] Create mirror service (to mirror current server in a another server - a parent server). This mirror server should prevent 'back notification' from the remote server.
[✔] Analyse the possibility of write var files in paralel (to best performance/best latency)
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