# ServerDemon
Automated game server manager for low-resource Linux machines. 
<br>
<br>
ServerDemon is designed to manage and run multiple game servers on underwhelming hardware, making it possible to rent a low-cost VPS to host concurrently active servers— autonomously. 
<br>
## Features
+ Automatically start and stop server software based on established connections: <br>
*ServerDemon will wait for connections on a specified port, and on receiving connections will hand the port back to the OS and run the server software. This works both ways, if the server software is running and no connections are present, the server software will be shutdown.*

## Dependencies
+ [inih (C++ version)](https://github.com/jtilly/inih)

## Usage
+ Add server configuration files
+ List the filepath of each configuration file in the "include" file
+ Run the makefile to build, then run serverdemon

## Server Configuration File Cheat Sheet
| Key | Value | Requirement |
| --- | ----- | ----------- |
| SERVER_NAME | A name for the server | Optional |
| EXE_PATH | Server executable filepath | Required |
| EXE_ARGS | Executable arguments to pass to the server | Optional |
| SERVER_ROOT | Root directory of the server program | Required |
| PORT | Server port number | Required |
| MAX_RAM | Maximum memory allowed for server program when multiple servers run concurrently (not yet implemented) | Optional |

## Planned Features
+ Daemonize
+ Memory throttling when running multiple servers
+ Self testing and validation
+ Plan more features
