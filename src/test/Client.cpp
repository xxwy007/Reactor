#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

#include "../common/Buffer.h"
#include "../common/RpcMessage.h"

int main()
{
    constexpr int CLIENT_NUM = 2;

    std::vector<int> fds;

    for (int i = 0; i < CLIENT_NUM; ++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd < 0)
        {
            perror("socket");
            continue;
        }

        sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = htons(9999);
        inet_pton(AF_INET,
                  "127.0.0.1",
                  &server.sin_addr);

        if (connect(fd,
                    (sockaddr *)&server,
                    sizeof(server)) < 0)
        {
            perror("connect");
            close(fd);
            continue;
        }

        fds.push_back(fd);

        if (i % 100 == 0)
        {
            std::cout
                << "connected: "
                << i
                << std::endl;
        }
    }

    std::cout
        << "total connected: "
        << fds.size()
        << std::endl;

    for (auto fd : fds)
    {
        RpcMessage msg1{"UserService", "queryAge", std::to_string(fd)};
        RpcMessage msg2{"OrderService", "queryOrder", std::to_string(fd)};
        RpcMessage msg;
        if (fd % 2 == 0)
            msg = msg1;
        else
            msg = msg2;
        Buffer buffer;
        std::string body = msg.service + "|" + msg.method + "|" + msg.body;
        uint32_t len = body.size();
        RpcHeader header{len, 100 + static_cast<uint64_t>(fd)};
        RpcPacket rpcpacket(header, msg);
        RpcCodec::encode(rpcpacket, buffer);

        std::vector<char> packet;
        packet.resize(buffer.readableBytes());

        memcpy(packet.data(), buffer.peek(), buffer.readableBytes());

        send(fd, packet.data(), packet.size(), 0);
    }

    char buf[1024];

    for (auto fd : fds)
    {
        int n = recv(fd, buf, sizeof(buf), 0);

        Buffer buffer;
        buffer.append(buf, n);
        std::cout
            << "recv bytes = "
            << n << std::endl;

        uint32_t len;

        RpcPacket packet;
        RpcCodec::decode(buffer, packet);

        std::cout << "msgid = " << packet.header.msgId << std::endl;
        std::cout << "body = " << packet.msg.body << std::endl;

        printf("\n");
    }

    std::cout << "press enter to exit";
    std::cin.get();

    for (auto fd : fds)
    {
        close(fd);
    }

    return 0;
}