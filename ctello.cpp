#include "ctello.h"

#include <sstream>

namespace
{
// Binds the given socket file descriptor ot the given port.
// Returns whether it suceeds or not and the error message.
std::pair<bool, std::string> BindSocketToPort(const int sockfd, const int port)
{
    sockaddr_in listen_addr{};
    listen_addr.sin_port = static_cast<in_port_t>(port);
    listen_addr.sin_family = AF_INET;
    int result = bind(sockfd, reinterpret_cast<sockaddr*>(&listen_addr),
                      sizeof(listen_addr));

    if (result == -1)
    {
        std::stringstream ss;
        ss << "bind: " << errno;
        ss << " (" << strerror(errno) << ")";
        return {false, ss.str()};
    }

    return {true, ""};
}

// Finds the destination socket address to talk to the given ip:port.
// Returns whether it suceeds or not and the error message.
std::pair<bool, std::string> FindDestinationAddr(
    const char* const ip,
    const char* const to_port,
    sockaddr_storage* const dest_addr)
{
    addrinfo* result_list{nullptr};
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int result = getaddrinfo(ip, to_port, &hints, &result_list);

    if (result != 0)
    {
        std::stringstream ss;
        ss << "getaddrinfo: " << result;
        ss << " (" << gai_strerror(result) << ") ";
        return {false, ss.str()};
    }

    memcpy(dest_addr, result_list->ai_addr, result_list->ai_addrlen);
    freeaddrinfo(result_list);

    return {true, ""};
}

// Sends a string of bytes to the given destination address.
// Returns the number of sent bytes and, if -1, the error message.
std::pair<int, std::string> SendTo(const int sockfd,
                                   sockaddr_storage& dest_addr,
                                   const std::string& command)
{
    const socklen_t addr_len{sizeof(dest_addr)};
    int result =
        sendto(sockfd, command.c_str(), command.size(), 0,
               reinterpret_cast<sockaddr*>(&dest_addr), addr_len);

    if (result == -1)
    {
        std::stringstream ss;
        ss << "sendto: " << errno;
        ss << " (" << strerror(errno) << ")";
        return {false, ss.str()};
    }

    return {true, ""};
}

// Receives a text response from the given destination address.
// Returns the number of received bytes and, if -1, the error message.
std::pair<int, std::string> ReceiveFrom(const int sockfd,
                                        sockaddr_storage& dest_addr,
                                        std::string& response)
{
    socklen_t addr_len{sizeof(dest_addr)};
    const size_t buffer_size{100};
    char buffer[buffer_size]{'\0'};
    // MSG_DONTWAIT -> Non-blocking
    // recvfrom is storing (re-populating) the sender address in dest_addr.
    int result = recvfrom(sockfd, buffer, buffer_size, MSG_DONTWAIT,
                          reinterpret_cast<sockaddr*>(&dest_addr), &addr_len);
    if (result == -1)
    {
        std::stringstream ss;
        ss << "recvfrom: " << errno;
        ss << " (" << strerror(errno) << ")";
        return {false, ss.str()};
    }

    response = buffer;

    return {true, ""};
}
}  // namespace

namespace ctello
{
Tello::Tello()
{
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

bool Tello::Bind()
{
    auto result = ::BindSocketToPort(m_sockfd, FROM_PORT);
    if (!result.first)
    {
        std::cout << "CTello Error: " << result.second << std::endl;
        return false;
    }

    result = ::FindDestinationAddr(IP, TO_PORT, &m_dest_addr);
    if (!result.first)
    {
        std::cout << "CTello Error: " << result.second << std::endl;
        return false;
    }

    std::cout << "Finding Tello ..." << std::endl;
    FindTello();
    std::cout << "Entered SDK mode" << std::endl;

    ShowTelloInfo();

    return true;
}

void Tello::FindTello()
{
    do
    {
        SendCommand("command");
        sleep(1);
    } while (!(ReceiveResponse()));
}

void Tello::ShowTelloInfo()
{
    std::optional<std::string> response{};

    SendCommand("sn?");
    while (!(response = ReceiveResponse()))
        ;
    std::cout << "Serial Number:\t" << *response << std::endl;

    SendCommand("sdk?");
    while (!(response = ReceiveResponse()))
        ;
    std::cout << "Tello SDK:\t" << *response << std::endl;

    SendCommand("wifi?");
    while (!(response = ReceiveResponse()))
        ;
    std::cout << "Wi-Fi Signal:\t" << *response << std::endl;

    SendCommand("battery?");
    while (!(response = ReceiveResponse()))
        ;
    std::cout << "Battery:\t" << *response << std::endl;
}

bool Tello::SendCommand(const std::string& command)
{
    auto result = ::SendTo(m_sockfd, m_dest_addr, command);
    if (result.first == -1)
    {
        std::cout << "CTello Error: " << result.second << std::endl;
        return false;
    }
    return true;
}

std::optional<std::string> Tello::ReceiveResponse()
{
    std::string response;
    auto result = ::ReceiveFrom(m_sockfd, m_dest_addr, response);
    if (result.first == -1)
    {
        std::cout << "CTello Error: " << result.second << std::endl;
        return {};
    }
    if (result.first == 0)
    {
        return {};
    }
    response.erase(response.find_last_not_of(" \n\r\t") + 1);
    return response;
}
}  // namespace ctello
