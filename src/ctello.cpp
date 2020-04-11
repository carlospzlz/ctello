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
// Reads the spdlog level from given environment variable name.
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
        // Info is default
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
    int result = sendto(sockfd, command.c_str(), command.size(), 0,
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
    spdlog::set_pattern(LOG_PATTERN);
    auto log_level = ::GetLogLevelFromEnv("SPDLOG_LEVEL");
    spdlog::set_level(log_level);
}

Tello::~Tello()
{
    close(m_sockfd);
}

bool Tello::Bind()
{
    auto result = ::BindSocketToPort(m_sockfd, FROM_PORT);
    if (!result.first)
    {
        spdlog::error(result.second);
        return false;
    }

    result = ::FindDestinationAddr(IP, TO_PORT, &m_dest_addr);
    if (!result.first)
    {
        spdlog::error(result.second);
        return false;
    }

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
    std::optional<std::string> response{};

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
    auto result = ::SendTo(m_sockfd, m_dest_addr, command);
    if (result.first == -1)
    {
        spdlog::error(result.second);
        return false;
    }
    spdlog::debug("Sent command to {0}:{1}: {2}", IP, TO_PORT, command);
    return true;
}

std::optional<std::string> Tello::ReceiveResponse()
{
    std::string response;
    auto result = ::ReceiveFrom(m_sockfd, m_dest_addr, response);
    if (result.first == -1)
    {
        spdlog::error(result.second);
        return {};
    }
    if (result.first == 0)
    {
        return {};
    }
    response.erase(response.find_last_not_of(" \n\r\t") + 1);
    spdlog::debug("Received response from {0}:{1}: {2}", IP, TO_PORT, response);
    return response;
}
}  // namespace ctello
