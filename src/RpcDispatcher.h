#ifndef RPC_DISPATCHER
#define RPC_DISPATCHER
#include <string>
#include <unordered_map>
#include <memory>
#include "common/Service.h"

class RpcPacket;
class RpcDispatcher
{
public:
    RpcDispatcher() = default;
    ~RpcDispatcher()
    {
    }
    void registerService(const std::string &name, std::shared_ptr<Service> service);

    RpcPacket dispatch(const RpcPacket &req);

private:
    std::unordered_map<std::string, std::shared_ptr<Service>> m_services;
};



#endif