#include "RpcDispatcher.h"
#include "common/RpcMessage.h"
void RpcDispatcher::registerService(const std::string &name, std::shared_ptr<Service> service)
{
    m_services[name] = service;
}

RpcPacket RpcDispatcher::dispatch(const RpcPacket &req)
{
    RpcPacket rsp;

    auto it = m_services.find(req.msg.service);

    if (it == m_services.end())
    {
        rsp.msg.body = "service not found";

        return rsp;
    }

    rsp.msg.body = it->second->call(req.msg.method, req.msg.body);

    return rsp;
}