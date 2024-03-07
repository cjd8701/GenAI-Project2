# GenAI-Project2: C2 Server and Client
## Goal
Produce a piece of software that is useful within the cybersecurity context. Leverage generative AI, showcase the ability and limitations of current tools.

## Overview 
This is a C2 server and persistent client that can execute several commands on the victim Windows machine.

## C2 Server
The server listens for connections from the client. It will then send commands for the client to execute and receive responses from the client. 

### Compiling
To compile the server run the included `compile.bat` script

### Commands (case sensitive)
- hello: Test command, client will return "world"
- keylog: Client will log keypresses for thirty seconds and return them to the server
- cleanup: Client will delete itself and cover up its existence. Registry key will be deleted and the process will delete the exe and close itself.
- Any other commands sent will be executed by a pipe. IE: sending `cmd /c dir` will execute a dir from command prompt in the current directory. This can also be used to execute any other program. 

## Client
Client receives commands from the C2 and performs actions. Different functions are contained in modules that are imported and more can be easily added to the client. The client will make itself persistent on the 
victim machine via a windows registry key.

### Compiling
To compile the client run the included `compile.bat` script

### Adding New Functions
New functions can be added via new .cpp files very easily. Simply import modules.h to the new .cpp file and add function declaration to it and then update client.cpp
to execute the functions.
