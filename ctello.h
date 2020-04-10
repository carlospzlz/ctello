#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <memory.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

// TODO
// Use spdlog

const char* const IP{"192.168.10.1"};
const char* const PORT{"8889"};

namespace ctello
{
class Tello
{
public:
    Tello();
    bool Bind();
    bool SendCommand(const std::string& command);
    bool ReceiveResponse() { return false; }
    // WaitForCommand() {}

private:
    int m_sockfd{0};
    sockaddr_storage m_dest_addr{};
};
}  // namespace ctello

