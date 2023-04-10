#include "GameServer.h"
#include "Util/Log.h"
#include "inih/INIReader.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>
#include <signal.h>
#include <fcntl.h>
#include <sstream>

#define TIME_WAIT_SECONDS 30


bool GameServer::LoadFromFile(const char* filepath) {
    Log("Loading server file: \"" << filepath << "\"");

    INIReader reader(filepath);

    if (reader.ParseError() != 0) {
        Log("ERROR: Failed to open server file: \"" << filepath << "\"");
        return false;
    }

    if ((m_executableFilePath = reader.Get("", "EXE_PATH", "FAIL")) == "FAIL") {
        Log("ERROR: Value EXE_PATH has not been set!");
        m_executableFilePath.clear();
        return false;
    }

    if ((m_serverRootDir = reader.Get("", "SERVER_ROOT", "FAIL")) == "FAIL") {
        Log("ERROR: Value SERVER_ROOT has not been set!");
        m_serverRootDir.clear();
        return false;
    }

    if ((m_port = reader.GetInteger("", "PORT", 0)) == 0) {
        Log("ERROR: Value PORT has not been set!");
        return false;
    }

    if ((m_name = reader.Get("", "SERVER_NAME", "Unnamed Game Server")) == "Unnamed Game Server") {
        Log("WARN: Value SERVER_NAME has not been set! Setting name to \"Unnamed Game Server\"");
    }

    if ((m_executableArgs = reader.Get("", "EXE_ARGS", "FAIL")) == "FAIL") {
        Log("WARN: Value EXE_ARGS has not beed set");
        m_executableArgs.clear();
    }

    // todo: actually implement resource based balancing between servers
    if ((m_allocatedRAM = reader.GetInteger("", "MAX_RAM", 0)) == 0) {
        Log("WARN: Value MAX_RAM has not been set, load balancing features will not be available for this server!");
    }

    // Print info after loading
    Log("Loaded server file \"" << filepath << "\" successfully: \n" <<
        "Server Name: \"" << m_name << "\"\n" <<
        "Executable Path: \"" << m_executableFilePath << "\"\n" <<
        "Executable Args: \"" << m_executableArgs << "\"\n" <<
        "Server Root Directory: \"" << m_serverRootDir << "\"\n" <<
        "Port: " << m_port );

    return true;
}

void GameServer::Kill() {
    if (m_pid != 0) {
        int result = kill(m_pid, SIGTERM);

        if (result == EPERM) 
            PrintMsg("Failed to kill server process (PID: " + std::to_string(m_pid) + "): Invalid permissions!");
        else if (result == ESRCH) 
            PrintMsg("Failed to kill server process (PID: " + std::to_string(m_pid) + "): Invalid PID!");
        else {
            PrintMsg("Killed server process (PID: " + std::to_string(m_pid) + ")");
            m_pid = 0;
        }
    }
    else {
        PrintMsg("Failed to kill server process: No known PID!");
    }
}

// Listen for network connections on server's port. To be launched on another thread by ServerMaster
void GameServer::ListenForConnections() {
    
    // Set state to idle so ServerMaster ignores me while I wait
    m_state = STATE_IDLE;

	int server_socket, connected_socket;
    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // create socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        PrintMsg("LISTENER THREAD: Failed to create listening socket, bye!");
        return;
    }

    // force reuse port if it's still in TIME_WAIT state
    if (setsockopt(server_socket, SOL_SOCKET, 
                    SO_REUSEADDR | SO_REUSEPORT, &opt,
                    sizeof(opt))) {
        PrintMsg("LISTENER THREAD: Failed to set socket options, bye!");
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(m_port);

    // attach socket to port
    if (bind(server_socket, (sockaddr*)&address, sizeof(address)) < 0) {
        PrintMsg("LISTENER THREAD: Failed to bind port " + std::to_string(m_port) + ", trying again in 60 seconds...");
        sleep(TIME_WAIT_SECONDS);
        ListenForConnections(); // restart
        return;
    }
    
    // setup socket to listen for connections on port (second arg = tcp backlog)
    if (listen(server_socket, 3) < 0) {
        PrintMsg("LISTENER THREAD: Failed to listen, bye!");
        return;
    }

    PrintMsg("LISTENER THREAD: Listening for connections on port " + std::to_string(m_port) + "...");

    // Block the thread here and wait for connections to be accepted
    if ((connected_socket = accept(server_socket, (sockaddr*)&address,
        (socklen_t*)&addrlen)) < 0) {
        PrintMsg("LISTENER THREAD: Connection failed, bye!");
        return;
    } 
    
    PrintMsg("LISTENER THREAD: Connection(s) received, waiting for port unbinding...");

    // VERY IMPORTANT: CLOSE, don't SHUTDOWN the socket so it can be reused by the server program
    // shutdown(2) keeps the socket bound to be reused!!
    close(connected_socket);
    close(server_socket);

    sleep(TIME_WAIT_SECONDS);

    // Done waiting for connections, set state to wakeup so ServerMaster starts the server program
    m_state = STATE_WAKEUP;
}

// Execute server program and check if it executed
bool GameServer::ExecServer() {
    PrintMsg("Executing: \"" + m_executableFilePath + "\"");
    // pipefd[0] = read-end, pipefd[1] = write-end
    int pipefd[2]; 
    char buffer[8];

    pipe(pipefd);
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK); // Do not block the main thread when reading messages from pipe

    m_pid = fork();
    

    // Forked child process, attempt to execute server program and report back to parent via pipe
    if (m_pid == 0) {
        close(pipefd[0]); // read end of pipe will not be used for child process
        
        // tokenize argument string into null terminated array for execv
        std::vector<const char*> args;

        std::stringstream ss;
        ss.str(m_executableArgs);

        std::string buffer;
        while(getline(ss, buffer, ' ')) {
            char* arg = strdup(buffer.c_str()); // don't bother freeing, child process will be replaced or terminated soon
            args.push_back(arg);
        }

        args.push_back(NULL);

        chdir(m_serverRootDir.c_str()); 
        
        // execv requires char pointers to be constant... so cast vector args accordingly
        if (execv(m_executableFilePath.c_str(), const_cast<char* const*>(args.data())) != 0) {
            write(pipefd[1], "FAIL", 5);
            exit(1);
        }
        
        close(pipefd[1]);
        exit(0); // Child process ran server and completed successfully
    }

    // Forked parent process, wait for response from child
    else {
        close(pipefd[1]); // write end of pipe will not be used for parent process

        read(pipefd[0], &buffer, 5); 

        if (strcmp(buffer, "FAIL") == 0) {
            PrintMsg("Failed to run server program!");
            close(pipefd[0]);
        }
        else {
            m_bootTimestamp = time(NULL); // Set this so ServerMaster doesn't immediately close the program
            close(pipefd[0]);
            return true; // Child process did not send fail message, everything should be OK
        }
    }

    return false;
}

// Print to console with server tag
void GameServer::PrintMsg(const std::string& msg) {
    std::cout << GetTimeF() << "[ " << m_name << " ] " << msg << '\n';
}

// Get established connections to server
int GameServer::GetConnections() {
    const std::string cmd = "netstat -anp 2>/dev/null | grep :" + std::to_string(m_port) + " | grep ESTABLISHED | wc -l";
    char connections[8];

    FILE* fp;
    fp = popen(cmd.c_str(), "r");
    fgets(connections, 8, fp);
    pclose(fp);

    return std::stoi(connections);
}