//  CTello is a C++ library to interact with the DJI Ryze Tello Drone
//  Copyright (C) 2020 Carlos Perez-Lopez
//
//  This library is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>
//
//  You can contact the author via carlospzlz@gmail.com

#include "ctello.h"

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

#include <sstream>

#include "spdlog/spdlog.h"

const char* const LOG_PATTERN = "[%D %T] [ctello] [%^%l%$] %v";

namespace
{
// Reads the spdlog level from the given environment variable name.
spdlog::level::level_enum GetLogLevelFromEnv(const std::string& var_name)
{
    // clang-format off
    std::unordered_map<std::string, spdlog::level::level_enum> name_to_enum = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}
    };
    // clang-format on
    const char* const name_c_str = std::getenv(var_name.c_str());
    if (name_c_str == nullptr)
    {
        // Info is the default
        return spdlog::level::info;
    }
    const std::string name{name_c_str};
    return name_to_enum[name];
}

// Binds the given socket file descriptor ot the given port.
// Returns whether it suceeds or not and the error message.
std::pair<bool, std::string> BindSocketToPort(const int sockfd, const int port)
{
    sockaddr_in listen_addr{};
    // htons converts from host byte order to network byte order.
    listen_addr.sin_port = htons(port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_family = AF_INET;
    int result = bind(sockfd, reinterpret_cast<sockaddr*>(&listen_addr),
                      sizeof(listen_addr));

    if (result == -1)
    {
        std::stringstream ss;
        ss << "bind to " << port << ": " << errno;
        ss << " (" << strerror(errno) << ")";
        return {false, ss.str()};
    }

    return {true, ""};
}

// Finds the socket address given an ip and a port.
// Returns whether it suceeds or not and the error message.
std::pair<bool, std::string> FindSocketAddr(const char* const ip,
                                            const char* const port,
                                            sockaddr_storage* const addr)
{
    addrinfo* result_list{nullptr};
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int result = getaddrinfo(ip, port, &hints, &result_list);

    if (result != 0)
    {
        std::stringstream ss;
        ss << "getaddrinfo: " << result;
        ss << " (" << gai_strerror(result) << ") ";
        return {false, ss.str()};
    }

    memcpy(addr, result_list->ai_addr, result_list->ai_addrlen);
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
    int result = sendto(sockfd, command.c_str(), command.size(), 0,
                        reinterpret_cast<sockaddr*>(&dest_addr), addr_len);

    if (result == -1)
    {
        std::stringstream ss;
        ss << "sendto: " << errno;
        ss << " (" << strerror(errno) << ")";
        return {-1, ss.str()};
    }

    return {result, ""};
}

// Receives a text response from the given destination address.
// Returns the number of received bytes and, if -1, the error message.
std::pair<int, std::string> ReceiveFrom(const int sockfd,
                                        sockaddr_storage& addr,
                                        std::string& response)
{
    socklen_t addr_len{sizeof(addr)};
    const size_t buffer_size{1024};
    char buffer[buffer_size]{'\0'};
    // MSG_DONTWAIT -> Non-blocking
    // recvfrom is storing (re-populating) the sender address in addr.
    int result = recvfrom(sockfd, buffer, buffer_size, MSG_DONTWAIT,
                          reinterpret_cast<sockaddr*>(&addr), &addr_len);
    if (result == -1)
    {
        std::stringstream ss;
        ss << "recvfrom: " << errno;
        ss << " (" << strerror(errno) << ")";
        return {-1, ss.str()};
    }

    response = buffer;

    return {result, ""};
}
}  // namespace

namespace ctello
{
Tello::Tello()
{
    m_command_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    m_state_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    m_stream_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    spdlog::set_pattern(LOG_PATTERN);
    auto log_level = ::GetLogLevelFromEnv("SPDLOG_LEVEL");
    spdlog::set_level(log_level);
}

Tello::~Tello()
{
    close(m_command_sockfd);
    close(m_state_sockfd);
    close(m_stream_sockfd);
}

bool Tello::Bind(const int local_client_command_port)
{
    // UDP Client to send commands and receive responses
    auto result =
        ::BindSocketToPort(m_command_sockfd, local_client_command_port);
    if (!result.first)
    {
        spdlog::error(result.second);
        return false;
    }
    m_local_client_command_port = local_client_command_port;
    result = ::FindSocketAddr(TELLO_SERVER_IP, TELLO_SERVER_COMMAND_PORT,
                              &m_tello_server_command_addr);
    if (!result.first)
    {
        spdlog::error(result.second);
        return false;
    }

    // Local UDP Server to listen for the Tello Status
    result = ::BindSocketToPort(m_state_sockfd, LOCAL_SERVER_STATE_PORT);
    if (!result.first)
    {
        spdlog::error(result.second);
        return false;
    }

    // Local UDP Server to listen for the Tello Video Stream
    result = ::BindSocketToPort(m_stream_sockfd, LOCAL_SERVER_STREAM_PORT);
    if (!result.first)
    {
        spdlog::error(result.second);
        return false;
    }

    // Finding Tello
    spdlog::info("Finding Tello ...");
    FindTello();
    spdlog::info("Entered SDK mode");

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
    std::optional<std::string> response;

    SendCommand("sn?");
    while (!(response = ReceiveResponse()))
        ;
    spdlog::info("Serial Number: {0}", *response);

    SendCommand("sdk?");
    while (!(response = ReceiveResponse()))
        ;
    spdlog::info("Tello SDK:     {0}", *response);

    SendCommand("wifi?");
    while (!(response = ReceiveResponse()))
        ;
    spdlog::info("Wi-Fi Signal:  {0}", *response);

    SendCommand("battery?");
    while (!(response = ReceiveResponse()))
        ;
    spdlog::info("Battery:       {0}", *response);
}

bool Tello::SendCommand(const std::string& command)
{
    auto result =
        ::SendTo(m_command_sockfd, m_tello_server_command_addr, command);
    const int bytes{result.first};
    if (bytes == -1)
    {
        spdlog::error(result.second);
        return false;
    }
    spdlog::debug("127.0.0.1:{} >>>> {} bytes >>>> {}:{}: {}",
                  m_local_client_command_port, bytes, TELLO_SERVER_IP,
                  TELLO_SERVER_COMMAND_PORT, command);
    return true;
}

std::optional<std::string> Tello::ReceiveResponse()
{
    std::string response;
    auto result =
        ::ReceiveFrom(m_command_sockfd, m_tello_server_command_addr, response);
    const int bytes{result.first};
    if (bytes < 1)
    {
        return {};
    }
    response.erase(response.find_last_not_of(" \n\r\t") + 1);
    spdlog::debug("127.0.0.1:{} <<<< {} bytes <<<< {}:{}: {}",
                  m_local_client_command_port, bytes, TELLO_SERVER_IP,
                  TELLO_SERVER_COMMAND_PORT, response);
    return response;
}

std::optional<std::string> Tello::GetState()
{
    std::string response;
    sockaddr_storage addr;
    auto result = ::ReceiveFrom(m_state_sockfd, addr, response);
    const int bytes{result.first};
    if (bytes < 1)
    {
        return {};
    }
    response.erase(response.find_last_not_of(" \n\r\t") + 1);
    spdlog::debug("127.0.0.1:{} <<<< {} bytes <<<< {}:{}: {}",
                  m_local_client_command_port, bytes, TELLO_SERVER_IP,
                  TELLO_SERVER_COMMAND_PORT, response);
    return response;
}

void Tello::GetFrame()
{
    std::string response;
    sockaddr_storage addr;
    auto result = ::ReceiveFrom(m_stream_sockfd, addr, response);
    const int bytes{result.first};
    if (bytes < 1)
    {
        return;
    }
    response.erase(response.find_last_not_of(" \n\r\t") + 1);
    spdlog::debug("127.0.0.1:{} <<<< {} bytes <<<< {}:{}: {}",
                  m_local_client_command_port, bytes, TELLO_SERVER_IP,
                  TELLO_SERVER_COMMAND_PORT, response);
}
}  // namespace ctello
