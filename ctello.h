#include <sys/socket.h>
#include <sys/types.h>

#include <optional>

// TODO
// Use spdlog

const char* const IP{"192.168.10.1"};
const char* const TO_PORT{"8889"};
const int FROM_PORT{9000};

namespace ctello
{
class Tello
{
public:
    Tello();
    bool Bind();
    bool SendCommand(const std::string& command);
    std::optional<std::string> ReceiveResponse();

private:
    void FindTello();
    void ShowTelloInfo();

private:
    int m_sockfd{0};
    sockaddr_storage m_dest_addr{};
};
}  // namespace ctello

