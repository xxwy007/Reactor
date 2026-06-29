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
    constexpr int CLIENT_NUM = 10;

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

        RpcPacket req1{RpcHeader{std::to_string(msg1), std::string("100" + std::to_string(fd))}, msg1};

        RpcMessage req2{"OrderService", "queryOrder", std::to_string(fd)};

        RpcMessage req;
        if (fd % 2 == 0)
            req = req1;
        else
            req = req2;
        Buffer buffer;
        RpcCodec::encode(req, buffer);

        std::string body = req.service + "|" + req.method + "|" + req.body;
        uint32_t len = body.size();

        std::vector<char> packet;

        packet.resize(4 + len);

        memcpy(packet.data(), &len, 4);

        memcpy(packet.data() + 4, body.data(), len);

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
            << n
            << "---";

        uint32_t len;

        memcpy(
            &len,
            buffer.peek(),
            4);

        buffer.retrieve(4);

        std::string body(
            buffer.peek(),
            len);

        buffer.retrieve(len);
        std::cout << "body = " << body << std::endl;

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