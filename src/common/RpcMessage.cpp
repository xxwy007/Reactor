#include "RpcMessage.h"
#include <algorithm>
#include <cstring>

bool RpcCodec::decode(Buffer &buffer, RpcPacket &packet)
{
    constexpr size_t HEADER_SIZE = sizeof(uint32_t) + sizeof(uint64_t);

    // Header都没收完整
    if (buffer.readableBytes() < HEADER_SIZE)
    {
        return false;
    }

    uint32_t bodyLen = 0;
    memcpy(&bodyLen, buffer.peek(), sizeof(uint32_t));
    // Body没收完整
    if (buffer.readableBytes() < HEADER_SIZE + bodyLen)
    {
        return false;
    }

    // 取header
    packet.header.bodyLen = bodyLen;
    buffer.retrieve(sizeof(uint32_t));

    uint32_t msgid = 0;
    memcpy(&msgid, buffer.peek(), sizeof(uint64_t));
    packet.header.msgId = msgid;
    buffer.retrieve(sizeof(uint64_t));

    // 取Body
    std::string body(buffer.peek(), bodyLen);
    buffer.retrieve(bodyLen);

    parseMessage(body, packet.msg);

    return true;
}

void RpcCodec::parseMessage(const std::string &body, RpcMessage &msg)
{
    auto pos1 = body.find('|');

    if (pos1 == std::string::npos)
    {
        return;
    }

    auto pos2 = body.find('|', pos1 + 1);

    if (pos2 == std::string::npos)
    {
        return;
    }

    msg.service = body.substr(0, pos1);

    msg.method = body.substr(pos1 + 1, pos2 - pos1 - 1);

    msg.body = body.substr(pos2 + 1);
}

void RpcCodec::encode(const RpcPacket &packet, Buffer &buffer)
{
    buffer.append(reinterpret_cast<const char *>(&packet.header.bodyLen), sizeof(uint32_t));
    buffer.append(reinterpret_cast<const char *>(&packet.header.msgId),sizeof(uint64_t));

    std::string body = packet.msg.service + "|" + packet.msg.method + "|" + packet.msg.body;
    uint32_t len = body.size();

    buffer.append(body.data(), len);
}
