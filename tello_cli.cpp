#include <iostream>
#include <thread>
#include <mutex>

#include "ctello.h"

using ctello::Tello;

const char* const PROMPT = "ctello> ";
// clang-format off
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
"  stop          Hovers in the air                                ok / error\n"
"  curve x1 y1 z1 x2 y2 z2 s  Fly describing a curve at speed 's' ok / error\n"
"\n"
"SET COMMANDS\n"
"  speed x       Set speed to 'x' cm/s [10-100]                   ok / error\n"
"  rc a b c      Set remote controller control via four channels  ok / error\n"
"  wifi s p      Set Wi-Fi 's' (ssid) to password 'p'             ok / error\n"
"  mon           Enable mission pad detection                     ok / error\n"
"  moff          Disable mission pad detection                    ok / error\n"
"  mdirection x  Set mission direction detection [0, 1, 2]        ok / error\n"
"  ap s p        Set the Tello to station mode                    ok / error\n"
"\n"
"READ COMMANDS\n"
"  speed?        Obtain current speed (cm/s)                      10 - 100  \n"
"  battery?      Obtain current battery percentage                0 - 100   \n"
"  time?         Obtain current flight time                       time      \n"
"  wifi?         Obtain Wi-Fi SNR                                 snr       \n"
"  sdk?          Obtain the Tello SDK version                     version   \n"
"  sn?           Obtain the Tello serial number                   serial    \n"
"\n";
// clang-format on

std::mutex mu;

class Listener
{
public:
    Listener(Tello& tello) : m_tello(tello) {}
    void Exit() { m_listening = false; std::cout << "set" << std::endl;}
    void operator()()
    {
        while (m_listening)
        {
            if (auto response = m_tello.ReceiveResponse())
            {
                mu.lock();
                std::cout << std::endl << *response << std::endl;
                mu.unlock();
            }
        }
        std::cout << "out" << std::endl;
    }

private:
    Tello& m_tello;
    bool m_listening{true};
};

int main()
{
    Tello tello{};
    if (!tello.Bind())
    {
        return 0;
    }

    Listener listener{tello};
    std::thread listeningThread(listener);

    std::string command{""};
    std::cout << PROMPT << std::flush;
    while (std::getline(std::cin, command))
    {
        if (command == "exit")
        {
            break;
        }
        if (command == "help")
        {
            std::cout << HELP << std::endl;
        }
        else if (command.size() > 0)
        {
            tello.SendCommand(command);
        }
        mu.lock();
        std::cout << PROMPT << std::flush;
        mu.unlock();
    }

    std::terminate(listeningThread);

    return 0;
}
