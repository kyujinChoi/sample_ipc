#include "IPCReader.h"
#include <signal.h>

IPCReader* ipc;

// using namespace umsg;


void sig_handler(int signo)
{
    ipc->Free();
    std::cout <<"signal: " << signo << std::endl;
    exit(0);
}

int main()
{
    coconut::coconut.cout("hello", coconut::Color::GREEN).endl();
    coconut::coconut.cout(359, coconut::Color::RED).endl().endl();
    coconut::coconut.cout(1453.687, coconut::Color::BLUE, coconut::Style::BOLD).endl(3);
    return 0;
}