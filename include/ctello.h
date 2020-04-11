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
    ~Tello();
    bool Bind();
    bool SendCommand(const std::string& command);
    std::optional<std::string> ReceiveResponse();

    Tello(const Tello&) = delete;
    Tello(const Tello&&) = delete;
    Tello& operator=(const Tello&) = delete;
    Tello& operator=(const Tello&&) = delete;

private:
    void FindTello();
    void ShowTelloInfo();

private:
    int m_sockfd{0};
    sockaddr_storage m_dest_addr{};
};
}  // namespace ctello

