#include "Reactor.h"
#include <iostream>

int main()
{
    Reactor reac;

    std::cout << "Server already\n";
    reac.addListenPort();
    reac.run();
    
    return 0;
}