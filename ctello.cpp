#include "ctello.h"

namespace ctello
{
Tello::Tello()
{
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

bool Tello::Bind()
{
    // Bind socket to a port
    sockaddr_in listen_addr{};
    listen_addr.sin_port = FROM_PORT;
    listen_addr.sin_family = AF_INET;
    int result = bind(m_sockfd, reinterpret_cast<sockaddr*>(&listen_addr),
                      sizeof(listen_addr));
    if (result == -1)
    {
        std::cout << "CTello Error: bind: " << errno << " (" << strerror(errno)
                  << ") " << std::endl;
        return false;
    }

    // Find destination address
    addrinfo* result_list{nullptr};
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    result = getaddrinfo(IP, TO_PORT, &hints, &result_list);
    if (result != 0)
    {
        std::cout << "CTello Error: getaddrinfo: " << result << " ("
                  << gai_strerror(result) << ")" << std::endl;
        return false;
    }
    memcpy(&m_dest_addr, result_list->ai_addr, result_list->ai_addrlen);
    freeaddrinfo(result_list);

    // Finding Tello
    // TODO Loop sending and listening without blocking (if possible)
    std::cout << "Finding Tello ..." << std::endl;
    SendCommand("command");
    if (auto response = ReceiveResponse())
    {
        std::cout << "Entered SDK mode" << std::endl;
    }

    info();

    return true;
}

void Tello::info()
{
    SendCommand("sn?");
    if (auto response = ReceiveResponse())
    {
        std::cout << "Serial Number:\t" << *response << std::endl;
    }
    SendCommand("sdk?");
    if (auto response = ReceiveResponse())
    {
        std::cout << "Tello SDK:\t" << *response << std::endl;
    }
    SendCommand("wifi?");
    if (auto response = ReceiveResponse())
    {
        std::cout << "Wi-Fi Signal:\t" << *response << std::endl;
    }
    SendCommand("battery?");
    if (auto response = ReceiveResponse())
    {
        std::cout << "Battery:\t" << *response << std::endl;
    }
}

bool Tello::SendCommand(const std::string& command)
{
    int result =
        sendto(m_sockfd, command.c_str(), command.size(), 0,
               reinterpret_cast<sockaddr*>(&m_dest_addr), sizeof(m_dest_addr));
    if (result == -1)
    {
    }
    return result != -1;
}

std::optional<std::string> Tello::ReceiveResponse()
{
    const size_t buffer_size{100};
    char buffer[buffer_size]{'\0'};
    socklen_t addr_len{sizeof(m_dest_addr)};
    int result = recvfrom(m_sockfd, buffer, buffer_size, 0,
                          reinterpret_cast<sockaddr*>(&m_dest_addr), &addr_len);
    if (result == -1)
    {
        // print error
        return {};
    }
    std::string response{buffer};
    response.erase(response.find_last_not_of(" \n\r\t") + 1);
    return response;
}
}  // namespace ctello
