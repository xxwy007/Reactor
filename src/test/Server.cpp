#include "../Reactor.h"
#include "../EventLoop.h"
#include "../RpcDispatcher.h"
#include "test.hpp"
#include <iostream>

int main()
{
    EventLoop loop;
    RpcDispatcher dispatcher = RpcDispatcher();
    dispatcher.registerService("UserService", std::make_shared<UserService>());
    dispatcher.registerService("OrderService", std::make_shared<OrderService>());
    Reactor reac(&loop, dispatcher);
    reac.addListenPort(9999);
    loop.loop();
    return 0;
}