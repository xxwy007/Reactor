#ifndef RPC_MESSAGE_H
#define RPC_MESSAGE_H
#include <cstdint>
#include <string>
#include "Buffer.h"

struct RpcHeader
{
    uint32_t bodyLen;
    uint64_t msgId;
};

struct RpcMessage
{
public:
    std::string service;
    std::string method;
    std::string body;
};

struct RpcPacket
{
    RpcHeader header;
    RpcMessage msg;
};

class RpcCodec
{
private:
    static void parseMessage(const std::string &body, RpcMessage &packet);

public:
    static bool decode(Buffer &buffer, RpcPacket &packet);

    static void encode(const RpcPacket &packet, Buffer &buffer);
};
#endif