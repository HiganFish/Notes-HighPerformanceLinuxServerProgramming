#include <csignal>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <cerrno>

#include "locker.h"
#include "httpconnection.h"
#include "threadpool.h"



constexpr int MAX_FD = 65535;
constexpr int MAX_EVENT_NUMBER = 10000;

extern int Addfd(int epollfd, int fd, bool one_shot);
extern int Removefd(int epollfd, int fd);

void AddSig(int sig, void(handler)(int), bool restart = true)
{
    struct sigaction sa{};
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void show_error(int connfd, const char *info)
{
    printf("%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("port need\n");
        return 1;
    }
    int port = atoi(argv[1]);

    AddSig(SIGPIPE, SIG_IGN);

    ThreadPool<HttpConnection> *pool = nullptr;
    try
    {
        pool = new ThreadPool<HttpConnection>;
    }
    catch (...)
    {
        return 1;
    }

    HttpConnection *users = new HttpConnection[MAX_FD];
    assert(users);
    int user_count = 0;

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    struct linger temp{1, 0};
    setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &temp, sizeof(temp));

    int ret = 0;
    struct sockaddr_in address{};
    address.sin_port = htons(port);
    address.sin_family = AF_INET;

    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret >= 0);

    ret = listen(listenfd, 5);

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    Addfd(epollfd, listenfd, false);
    HttpConnection::epollfd_ = epollfd;

    while (true)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            int what = events[i].events;
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_address{};
                socklen_t client_addresslen = sizeof(client_address);
                int connfd = accept(listenfd, (sockaddr*)&client_address, &client_addresslen);
                if (connfd < 0)
                {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if (HttpConnection::user_count_ >= MAX_FD)
                {
                    show_error(connfd, "Internal server busy");
                    continue;
                }
                users[connfd].Init(connfd, client_address);
            }
            else if (what & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                users[sockfd].CloseConn();
            }
            else if (what & EPOLLIN)
            {
                if (users[sockfd].Read())
                {
                    pool->Append(users + sockfd);
                }
                else
                {
                    users[sockfd].CloseConn();
                }
            }
            else if (what & EPOLLOUT)
            {
                if (!users[sockfd].Write())
                {
                    users[sockfd].CloseConn();
                }
            }
        }
    }
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
    return 0;
}
