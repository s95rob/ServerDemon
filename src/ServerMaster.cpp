#include "ServerMaster.h"
#include "Util/Log.h"
#include "Types.h"
#include <fstream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

void ServerMaster::Initialize() {
    Log("Initializing ServerMaster...");
    std::ifstream includedServersFile;
    const char* dev_includeFilepath = "../etc/serverdemon/include";
    const char* prod_includeFilepath = "/etc/serverdemon/include";

    // Read game server file paths from included servers file

    // Clear gameServers vector incase of reinitialize
    if (!m_gameServers.empty()) 
        m_gameServers.clear();

    // Priority load dev include_filepath if it exists
    if (access(dev_includeFilepath, F_OK) == 0) 
        includedServersFile = std::ifstream(dev_includeFilepath);
    else includedServersFile = std::ifstream(prod_includeFilepath);

    if (includedServersFile.fail()) {
        Log("Cannot find include file! Bye!");
        exit(1);
    }

    // Load GameServers from included servers file

    std::vector<std::string> includedServerPaths;
    std::string pathBuffer;

    while (getline(includedServersFile, pathBuffer)) {
        includedServerPaths.push_back(pathBuffer);
    }

    for (const auto& path : includedServerPaths) {
        GameServer newGameServer;
        if (newGameServer.LoadFromFile(path.c_str())) {
            m_gameServers.push_back(newGameServer);
        }
        else Log("Aborted loading server file: \"" << path << "\"");
    }

    // Close if no servers were successfully loaded
    if (m_gameServers.size() == 0) {
        Log("No server files were loaded! Bye!");
        exit(1);
    }
    else Log("ServerMaster initialized. Loaded " << m_gameServers.size() << " game " << (m_gameServers.size() > 1 ? "servers" : "server"));

    m_running = true;

    Runtime();
}

// Main program loop
void ServerMaster::Runtime() {
	Log("Starting ServerMaster runtime...\n");

    while (m_running) {
        //Log("ServerMaster runtime cycle start...");

		// Manage registered game servers
        for (auto& server : m_gameServers) {
            if (server.GetState() == STATE_OFF) {

                //     server state = OFF -> launch listen thread, thread sets server state = IDLE -v
                //  v-------------------------------------------------------------------------------<
                //  -> connection received from thread, thread closes and sets server state = WAKEUP

                std::thread listener(&GameServer::ListenForConnections, &server);
                listener.detach();
            }

            else if (server.GetState() == STATE_WAKEUP) {
				// If ExecServer succeeds, it will set its server's state to RUNNING
				if (server.ExecServer()) {
                    server.SetState(STATE_RUNNING);
                }
                else server.SetState(STATE_OFF); // try again
            }

			else if (server.GetState() == STATE_RUNNING) {
				if (server.GetPID() == 0)
					server.SetState(STATE_OFF);
				else {
					// Check up on the server program
					if (kill(server.GetPID(), 0) < 0) {
						server.PrintMsg("Lost track of PID, restarting...");
						server.SetState(STATE_OFF);
					}
                    else {
                        // After 5 minute grace period, check if players are still connected
                        if (server.GetConnections() < 1 && time(NULL) > server.GetTimestamp("BOOT") + (60 * 5)) {
                            server.PrintMsg("No players connected, closing server in 60 seconds...");
                            server.SetState(STATE_CLOSING);
                            server.SetTimestamp("CLOSE");
                        }
                    }
				}
			}

            else if (server.GetState() == STATE_CLOSING) {
                // Close server if closing grace period ended and no connections are present
                if (server.GetConnections() < 1 && time(NULL) > server.GetTimestamp("CLOSE") + 60) {
                    server.PrintMsg("Closing server now...");
                    server.Kill();
                    server.SetState(STATE_OFF);
                }
                // Players reconnected to server before server closed, abort
                else if(server.GetConnections() > 0 && time(NULL) < server.GetTimestamp("CLOSE") + 60) {
                    server.PrintMsg("Players connected, aborting server close");
                    server.SetState(STATE_RUNNING);
                }
            }
        }

        // Sleep for awhile...
        sleep(m_runtimeWaitSeconds);
    }
}