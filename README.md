# ServerDemon
Automated game server manager for low-resource Linux machines. 
<br>
<br>
ServerDemon is a hopeful answer to a problem where a group of friends wanted dedicated servers for various games they play, but also wanted to spend the least amount of money each month on a VPS to host them. The tradeoff between value and performance when renting a VPS is stark, and running multiple servers concurrently would be too expensive of a task for a single, modestly endowed (virtual) machine. Performance needs to be lean and nimble. I also did not want to spend a whole lot of time maintaining a server, so a convenient, set-and-forget design was kept first in mind. There are existing game server managing solutions out there, but none of them to my knowledge were designed for a singular host with underwhelming specs. 
<br>
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
| EXE_PATH | Filepath to the server program | Required |
| EXE_ARGS | Arguments you would pass when running the program from a terminal or script | Optional |
| SERVER_ROOT | Root directory of the server program | Required |
| PORT | Port number the server will run on | Required |
| MAX_RAM | Maximum memory allowed for server program when multiple servers run concurrently (Coming soon) | Optional |

## Planned Features
+ Run as a daemon (ServerD*a*emon) and add install script
+ Server software memory throttling between multiple running servers
+ Self testing and validation
+ Plan more features
