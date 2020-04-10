#include "ctello.h"

using ctello::Tello;

const char* const PROMPT = "ctello> ";
const char* const HELP =
"The Tello SDK includes three basic command types.\n"
"\n"
"  Control Commands (xxx)\n"
"   - Returns “ok” if the command was successful.\n"
"   - Returns “error” or an informational result code if the command failed.\n"
"\n"
"  Set Command (xxx a) to set new sub-parameter values\n"
"   - Returns “ok” if the command was successful.\n"
"   - Returns “error” or an informational result code if the command failed.\n"
"\n"
"  Read Commands (xxx?)\n"
"   - Returns the current value of the sub-parameters.\n"
"\n"
"CONTROL COMMANDS\n"
"  command       Enter SDK mode                                   ok / error\n"
"  takeoff       Auto takeoff                                     ok / error\n"
"  land          Auto landing                                     ok / error\n"
"  streamon      Enable video stream                              ok / error\n"
"  streamoff     Disable video stream                             ok / error\n"
"  emergency     Stop motors immediately                          ok / error\n"
"  up x          Ascend to 'x' cm [20-500]                        ok / error\n"
"  down x        Descend to 'x' cm [20-500]                       ok / error\n"
"  left x        Fly left for 'x' cm [20-500]                     ok / error\n"
"  right x       Fly right for 'x' cm [20-500]                    ok / error\n"
"  forward x     Fly forward for 'x' cm [20-500]                  ok / error\n"
"  back x        Fly backward for 'x' cm [20-500]                 ok / error\n"
"  cw            Rotate 'x' degrees in clockwise [1-360]          ok / error\n"
"  ccw           Rotate 'x' degrees in counterclockwise [1-360]   ok / error\n"
"  flip x        Flip in 'x' direction [l, r, f, b]               ok / error\n"
"  go x y z s    Fly to (x, y, z) at speed 's' [10-100]           ok / error\n"
"  curve x1 y1 z1 x2 y2 z2 s  Fly describing a curve at speed 's' ok / error\n"
"\n"
"SET COMMANDS\n"

/*
“x” = 20-500
cw x Rotate “x” degrees clockwise.
“x” = 1-360
ccw x Rotate “x” degrees counterclockwise.
“x” = 1-360
flip x Flip in “x” direction.
“l” = left
“r” = right
“f” = forward
“b” = back
ok / error
Fly to “x” “y” “z” at “speed” (cm/s).
go x y z speed
“x” = -500-500
“y” = -500-500
“z” = -500-500
“speed” = 10-100
Note: “x”, “y”, and “z” values can’t be set between
-20 – 20 simultaneously.
© 2018 Ryze Tech. All Rights Reserved.
3Hovers in the air.
Note: works at any time.
stop
Fly at a curve according to the two given coordinates
at “speed” (cm/s).
If the arc radius is not within a range of 0.5-10 meters,
it will respond with an error.
curve x1 y1 z1 x2 y2
z2 speed
“x1”, “x2” = -500-500
“y1”, “y2” = -500-500
“z1”, “z2” = -500-500
“speed” = 10-60
Note: “x”, “y”, and “z” values can’t be set between
-20 – 20 simultaneously.
Fly to the “x”, “y”, and “z” coordinates of the Mission
Pad.
“mid” = m1-m8
“x” = -500-500
“y” = -500-500
“z” = -500-500
“speed” = 10-100 (cm/s)
go x y z speed mid
Note: “x”, “y”, and “z” values can’t be set between
-20 – 20 simultaneously.
Fly at a curve according to the two given coordinates
of the Mission Pad ID at “speed” (cm/s).
If the arc radius is not within a range of 0.5-10 meters,
it will respond with an error.
curve x1 y1 z1 x2 y2
z2 speed mid
“x1”, “x2” = -500-500
“y1”, “y2” = -500-500
“z1”, “z2” = -500-500
“speed” = 10-60
Note: “x”, “y”, and “z” values can’t be set between
-20 – 20 simultaneously.
Fly to coordinates “x”, “y”, and “z” of Mission Pad 1,
and recognize coordinates 0, 0, “z” of Mission Pad 2
and rotate to the yaw value.
“mid” = m1-m8
“x” = -500-500
“y” = -500-500
“z” = -500-500
“speed” = 10-100 (cm/s)
jump x y z speed yaw
mid1 mid2
Note: “x”, “y”, and “z” values can’t be set between
-20 – 20 simultaneously.


Set Commands
Command Command
speed x Set speed to “x” cm/s.
x = 10-100
Possible Response
Set remote controller control via four channels.
rc a b c d
“a” = left/right (-100-100)
“b” = forward/backward (-100-100)
“c” = up/down (-100-100)
“d” = yaw (-100-100)
Set Wi-Fi password.
wifi ssid pass
ssid = updated Wi-Fi name
pass = updated Wi-Fi password
mon Enable mission pad detection (both forward and
downward detection).
moff Disable mission pad detection.
“x” = 0/1/2
0 = Enable downward detection only
1 = Enable forward detection only
2 = Enable both forward and downward detection
mdirection x
ap ssid pass
ok / error
Notes:
Perform “mon” command before performing this
command.
The detection frequency is 20 Hz if only the forward
or downward detection is enabled. If both the forward
and downward detection are enabled, the detection
frequency is 10 Hz.
Set the Tello to station mode, and connect to a
new access point with the access point’s ssid and
password.
ssid = updated Wi-Fi name
pass = updated Wi-Fi password
Read Commands
Command
Command Possible Response
speed? Obtain current speed (cm/s). “x” = 10-100
battery? Obtain current battery percentage. “x” = 0-100
time? Obtain current flight time. “time”
wifi? Obtain Wi-Fi SNR. “snr”
© 2018 Ryze Tech. All Rights Reserved.
5sdk? Obtain the Tello SDK version. “sdk version”
sn? Obtain the Tello serial number. “serial number”
*/
"";

void ShowHelp()
{
}

int main()
{
    Tello tello{};
    if (!tello.Bind())
    {
        return 0;
    }

    std::string command{""};
    std::cout << PROMPT << std::flush;
    while (std::getline(std::cin, command))
    {
        if (command == "exit")
        {
            return 0;
        }
        if (command == "help")
        {
            std::cout << HELP << std::endl;
        }
        else if (command.size() > 0)
        {
            tello.SendCommand(command);
            if (auto response = tello.ReceiveResponse())
            {
                std::cout << *response << std::endl;
            }
        }
        std::cout << PROMPT << std::flush;
    }
    return 0;
}
