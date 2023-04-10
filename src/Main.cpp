#include "ServerMaster.h"
#include "Util/Log.h"
#include <unistd.h>

int main() {
    Log("ServerDemon started (PID: " << getpid() << ")");

    ServerMaster master;

    return 0;
}