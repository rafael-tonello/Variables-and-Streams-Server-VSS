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

[ ] Variable persistency
[ ] Configuration system (to specify)
[ ] UDP replay to server search
[ ] Restfull and ws API
[ ] Rename Controller to Business or Logic (to analyze. Is to prevent confuse with MVC system)
[ ] Create a logger library
[ ] Create the configuration system (observable configuration system)
[ ] Paralel project (app server)