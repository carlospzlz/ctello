#include "ctello.h"

using ctello::Tello;

int main()
{
    Tello tello{};
    tello.Bind();

    tello.SendCommand("command");
    tello.SendCommand("takeoff");
    sleep(3);
    tello.SendCommand("flip b");
    sleep(3);
    tello.SendCommand("flip f");
    sleep(3);
    tello.SendCommand("flip l");
    sleep(3);
    tello.SendCommand("flip r");
    sleep(3);
    tello.SendCommand("land");

    return 0;
}
