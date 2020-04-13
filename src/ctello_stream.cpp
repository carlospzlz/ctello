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

#include <cstdlib>
#include <iomanip>
#include <iostream>

#include "ctello.h"

using ctello::Tello;

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

    while (true)
    {
        if (const auto frame_opt = tello.GetFrame())
        {
            auto frame = *frame_opt;
            std::cout << "Frame of " << frame.size() << std::endl;
        }
    }
}
