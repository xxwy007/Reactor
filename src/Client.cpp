#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

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
                    (sockaddr*)&server,
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

    const char* msg = "hello reactor";

    for (auto fd : fds)
    {
        send(fd,
             msg,
             strlen(msg),
             0);
    }

    char buffer[1024];

    for (auto fd : fds)
    {
        int n = recv(fd,
                     buffer,
                     sizeof(buffer) - 1,
                     0);

        if (n > 0)
        {
            buffer[n] = '\0';

            std::cout
                << fd
                << " recv: "
                << buffer
                << std::endl;
        }
    }

    std::cout << "press enter to exit";
    std::cin.get();

    for (auto fd : fds)
    {
        close(fd);
    }

    return 0;
}