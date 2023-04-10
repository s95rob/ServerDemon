#pragma once

#include <string>
#include <vector>
#include "GameServer.h"

class ServerMaster {
public:
    ServerMaster() { Initialize(); }
    ~ServerMaster() = default;
    
    void Initialize();
    
private:
    void Runtime();

    bool m_running;
    std::vector<GameServer> m_gameServers;
    int m_runtimeWaitSeconds = 2;
    int m_msgRecvPort = 900;
};