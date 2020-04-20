//  CTello is a C++ library to interact with the DJI Ryze Tello Drone
//  Copyright (C) 2020 Carlos Perez-Lopez
//
//  This program is free software: you can redistribute it and/or modify
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

#include <iostream>

#include "ctello.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

const char* const TELLO_STREAM_URL{"udp://0.0.0.0:11111"};

using ctello::Tello;
using cv::CAP_FFMPEG;
using cv::imshow;
using cv::VideoCapture;
using cv::waitKey;

int main()
{
    Tello tello{};
    if (!tello.Bind())
    {
        return 0;
    }

    tello.SendCommand("streamon");
    while (!(tello.ReceiveResponse()))
        ;

    VideoCapture capture{TELLO_STREAM_URL, CAP_FFMPEG};
    std::array<std::string, 8> commands{"takeoff", "flip l", "flip r", "flip f",
                                        "flip b",  "stop",   "cw 360", "land"};
    int index{0};
    bool busy{false};
    while (true)
    {
        // See surrounding.
        cv::Mat frame;
        capture >> frame;

        // Listen response
        if (const auto response = tello.ReceiveResponse())
        {
            std::cout << "Tello: " << *response << std::endl;
            busy = false;
        }

        // Act
        if (!busy && index < commands.size())
        {
            const std::string command{commands[index++]};
            tello.SendCommand(command);
            std::cout << "Command: " << command << std::endl;
            busy = true;
        }

        // Show what the Tello sees
        imshow("CTello Stream", frame);
        if (waitKey(1) == 27)
        {
            break;
        }
    }
}
