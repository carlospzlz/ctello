#include "ctello.h"

using ctello::Tello;

const char* const PROMPT = "ctello> ";

void ShowHelp()
{

}

int main()
{
    Tello tello{};
    tello.Bind();

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
            ShowHelp();
        }
        else if (command.size() > 0)
        {
            std::cout << "tello" << std::endl;
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
