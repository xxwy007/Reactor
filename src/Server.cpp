#include "Reactor.h"
#include <iostream>

int main()
{
    EventLoop loop;
    Reactor reac(&loop);

    reac.addListenPort(9999);
    loop.loop();    
    return 0;
}