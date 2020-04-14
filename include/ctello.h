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

#include <sys/socket.h>
#include <sys/types.h>

#include <optional>
#include <vector>

// This is the server running in Tello, where we send commands to and we
// receive responses from
const char* const TELLO_SERVER_IP{"192.168.10.1"};
const char* const TELLO_SERVER_COMMAND_PORT{"8889"};

// This is the local port where we bind our local UDP client to.
//
// NOTE 1: With UDP, we have to bind() the socket in the client because UDP is
// connectionless, so there is no other way for the stack to know which
// program to deliver datagrams to for a particular port.
//
// NOTE 2: Tello will respond to any port, but it keeps that port number
// rememebered while it's powered up. So you can't have more than one client
// receiving responses from the Tello.
const int LOCAL_CLIENT_COMMAND_PORT{9000};

// We need to start a local UPD server to receive state updates.
const int LOCAL_SERVER_STATE_PORT{8890};

namespace ctello
{
class Tello
{
public:
    Tello();
    ~Tello();
    bool Bind(int local_client_command_port = LOCAL_CLIENT_COMMAND_PORT);
    bool SendCommand(const std::string& command);
    std::optional<std::string> ReceiveResponse();
    std::optional<std::string> GetState();

    Tello(const Tello&) = delete;
    Tello(const Tello&&) = delete;
    Tello& operator=(const Tello&) = delete;
    Tello& operator=(const Tello&&) = delete;

private:
    void FindTello();
    void ShowTelloInfo();

private:
    int m_command_sockfd{0};
    int m_state_sockfd{0};
    int m_stream_sockfd{0};
    int m_local_client_command_port{LOCAL_CLIENT_COMMAND_PORT};
    sockaddr_storage m_tello_server_command_addr{};
};
}  // namespace ctello

