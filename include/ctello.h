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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <thread>
#include "spdlog/spdlog.h"
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
    explicit Tello(bool withThreads);
    ~Tello();
    bool Bind(int local_client_command_port = LOCAL_CLIENT_COMMAND_PORT,
              int local_server_command_port = LOCAL_SERVER_STATE_PORT);
    int GetBatteryStatus();
    int GetHeightStatus();
    int GetHeightState(int amountOfTries = 20);
    std::string GetAccelerationStatus();
    double GetSpeedStatus();
    bool SendCommand(const std::string& command);
    bool SendCommandWithResponse(const std::string& command,
                                 int amountOfTries = 30000);

    bool SendCommandWithResponseByThread(const std::string& command,
                                         int amountOfTries = 500000);
    bool EasyLanding();
    std::string GetTelloName();
    std::optional<std::string> ReceiveResponse();
    std::optional<std::string> GetState();
    void createSockets();
    void closeSockets();
    /*Tello(const Tello&) = delete;
    Tello(const Tello&&) = delete;
    Tello& operator=(const Tello&) = delete;
    Tello& operator=(const Tello&&) = delete;*/
    void RcCommand(const std::string& rcCommand);
    int GetBatteryState(int amountOfTries = 20);
    int GetHeight() { return height; };
    int GetBattery() { return battery; };

private:
    void FindTello();
    void ShowTelloInfo();
    /*std::string logFileName= "";
    std::ofstream telloLogFile;*/
    int m_command_sockfd{0};
    int m_state_sockfd{0};
    int m_local_client_command_port{LOCAL_CLIENT_COMMAND_PORT};
    sockaddr_storage m_tello_server_command_addr{};
    int height;
    int battery;
    std::thread responseReceiver;
    std::thread stateReceiver;
    int timeBetweenRcCommandInMicroSeconds = 1000000;
    std::chrono::time_point<
        std::chrono::_V2::system_clock,
        std::chrono::duration<int64_t, std::ratio<1, 1000000000>>>
        lastTimeOfRCCommand;
    std::vector<std::string> responses;
    void listenToResponses();
    void listenToState();
    bool BindWithOutStatus(
        int local_client_command_port = LOCAL_CLIENT_COMMAND_PORT,
        int local_server_command_port = LOCAL_SERVER_STATE_PORT);
};
}  // namespace ctello
