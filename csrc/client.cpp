#include <iostream>
#include <thread>
#include <vector>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>

void worker(int idx)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);

    inet_pton(AF_INET,
              "127.0.0.1",
              &addr.sin_addr);

    connect(fd,
            (sockaddr*)&addr,
            sizeof(addr));

    char sendbuf[128];
    snprintf(sendbuf,
             sizeof(sendbuf),
             "client-%d",
             idx);

    send(fd,
         sendbuf,
         strlen(sendbuf),
         0);

    char recvbuf[128];

    recv(fd,
         recvbuf,
         sizeof(recvbuf),
         0);

    printf("recv:%s\n", recvbuf);

    close(fd);
}

int main()
{
    std::vector<std::thread> threads;

    for (int i = 0; i < 1000; i++)
    {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads)
    {
        t.join();
    }

    return 0;
}