//  CTello is a C++ library to interact with the DJI Ryze Tello Drone
//  Copyright (C) 2021 Carlos Perez-Lopez
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

#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <linux/joystick.h>

#include <iostream>
#include <optional>

#include "ctello.h"

static constexpr char INPUT_DEVICE_NAME[]{"/dev/input/js0"};
static constexpr int MAX_AXIS_VALUE{32767};
static constexpr int AXIS_SCALE{30};

using ctello::Tello;

class Joystick
{
public:
    Joystick(const std::string& input_device) : m_input_device(input_device)
    {
        m_fd = open(m_input_device.c_str(), O_RDONLY | O_NONBLOCK);
    }

    void ProcessEvents()
    {
        js_event event;
        while (read(m_fd, &event, sizeof(event)) != -1)
        {
            event.type &= ~JS_EVENT_INIT;

            if (event.type == JS_EVENT_BUTTON && event.value == 1)
            {
                ProcessButtonEvent(event.number);
            }
            else if (event.type == JS_EVENT_AXIS)
            {
                ProcessAxisEvent(event.number, event.value);
            }
        }
    }

    std::optional<std::string> GetCmd()
    {
        std::string cmd;
        if (!m_action_cmd.empty())
        {
            std::swap(m_action_cmd, cmd);
            return cmd;
        }

        if (!m_move_cmd.empty())
        {
            return m_move_cmd;
        }

        return {};
    }

private:
    enum DS4_BUTTONS
    {
        CROSS,
        CIRCLE,
        TRIANGLE,
        SQUARE,
        L1,
        R1,
        L2,
        R2,
    };

    enum DS4_AXIS
    {
        L3_X,
        L3_Y,
        L2_AXIS,
        R3_X,
        R3_Y,
    };

private:
    void ProcessButtonEvent(uint8_t number)
    {
        switch (number)
        {
        case DS4_BUTTONS::TRIANGLE:
        {
            m_action_cmd = "takeoff";
            break;
        }
        case DS4_BUTTONS::CROSS:
        {
            m_action_cmd = "land";
            break;
        }
        case DS4_BUTTONS::L1:
        {
            m_action_cmd = "flip l";
            break;
        }
        case DS4_BUTTONS::R1:
        {
            m_action_cmd = "flip r";
            break;
        }
        case DS4_BUTTONS::L2:
        {
            m_action_cmd = "flip b";
            break;
        }
        case DS4_BUTTONS::R2:
        {
            m_action_cmd = "flip f";
            break;
        }
        }
    }

    void ProcessAxisEvent(const uint8_t number, const int16_t value)
    {
        if (value == 0)
        {
            m_move_cmd.clear();
            return;
        }

        std::string cmd;
        switch (number)
        {
        case DS4_AXIS::L3_X:
        {
            cmd = value > 0 ? "cw" : "ccw";
            break;
        }
        case DS4_AXIS::L3_Y:
        {
            cmd = value > 0 ? "down" : "up";
            break;
        }
        case DS4_AXIS::R3_X:
        {
            cmd = value > 0 ? "right" : "left";
            break;
        }
        case DS4_AXIS::R3_Y:
        {
            cmd = value > 0 ? "back" : "forward";
            break;
        }
        }

        auto val = static_cast<int>(
            round(20 + fabs(value / float(MAX_AXIS_VALUE)) * AXIS_SCALE));
        m_move_cmd = cmd + " " + std::to_string(val);
    }

private:
    std::string m_input_device;
    int m_fd;
    std::string m_action_cmd;
    std::string m_move_cmd;
};

int main()
{
    Tello tello{};
    if (!tello.Bind())
    {
        return 0;
    }

    Joystick joystick{INPUT_DEVICE_NAME};
    while (true)
    {
        joystick.ProcessEvents();

        if (const auto cmd = joystick.GetCmd())
        {
            tello.SendCommand(*cmd);
        }

        // Sleep for 20ms
        usleep(2E4);
    }

    return 0;
}
