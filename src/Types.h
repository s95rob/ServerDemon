#pragma once

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string, std::string> GameSeverConfig;

enum GAMESERVER_STATE {
    STATE_OFF		= 0,
    STATE_IDLE		= 1,
    STATE_WAKEUP	= 2,
    STATE_RUNNING	= 3,
    STATE_CLOSING	= 4
};