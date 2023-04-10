#pragma once
#include <string>
#include <thread>
#include "Types.h"

#define PACKET_SAMPLE_SIZE 512

class GameServer {
public:
    friend class ListenerThread;

    GameServer() = default;
    ~GameServer() = default;

    bool LoadFromFile(const char* filepath);
    void PrintMsg(const std::string& msg);
    void ListenForConnections();
    bool ExecServer();
    void Kill();

    void SetState(GAMESERVER_STATE state) { m_state = state; }
    void SetTimestamp(const std::string& timestamp) {
        if (timestamp == "BOOT")        m_bootTimestamp = time(NULL);
        else if (timestamp == "CLOSE")  m_closeTimestamp = time(NULL);
        else PrintMsg("Attempted to set non-existant timestamp: " + timestamp);
    }

    const std::string& GetName()    { return m_name; }
    int GetPID()                    { return m_pid; }
    int GetNetFailCount()           { return m_netFailCount; }
    int GetPort()					{ return m_port; }
    int GetAllocatedRAM()			{ return m_allocatedRAM; }
    int GetConnections();
    GAMESERVER_STATE GetState()		{ return m_state; }
    time_t GetTimestamp(const std::string& timestamp) { 
        if (timestamp == "BOOT") return m_bootTimestamp;
        else if (timestamp == "CLOSE") return m_closeTimestamp;
        else {
            PrintMsg("Attempted to retrieve non-existant timestamp: " + timestamp);
            return NULL;
        } 
    }
    
private:
    bool Validate();

    std::string m_name, 
                m_executableFilePath,
                m_executableArgs,
                m_serverRootDir;
    
    int m_port = 0, 
        m_allocatedRAM = 0,
        m_netFailCount = 0;
    
    int m_pid = 0;

    time_t m_bootTimestamp = 0,
           m_closeTimestamp = 0;

    GAMESERVER_STATE m_state = STATE_OFF;
};