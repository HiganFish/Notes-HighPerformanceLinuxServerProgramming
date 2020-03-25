#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"

#define USER_LIMIT 5
#define BUFFER_SIZE 1024
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define PROCESS_LIMIT 65535

struct ClientData
{
    sockaddr_in address;
    int connfd;
    pid_t pid;
    int pipefd[2];
};

ClientData *users = nullptr;
int *pid_to_user_sub = nullptr;

static const char *shm_name = "/my_shm";
char *share_mem = nullptr;
int listenfd;
int epollfd;
int sig_pipe[2];
int shmfd;

int user_count = 0;

bool stop_child = false;
void ChildTermHandler(int sig)
{
    stop_child = true;
};

void SetNonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    fcntl(fd, old_option | O_NONBLOCK);
}

void Addfd(int epfd, int fd)
{
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    SetNonblock(fd);
}

void SignalHandler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipe[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void AddSignal(int sig, void (*handler)(int), bool restat = true)
{
    struct sigaction sa{};
    sa.sa_handler = handler;
    if (restat)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    int ret = sigaction(sig, &sa, nullptr);
    ERROR_IF(ret == -1, "add sig");
}

int RunChild(int idx, ClientData *the_users, char *the_share_mem)
{
    epoll_event events[MAX_EVENT_NUMBER]{};
    int child_epollfd = epoll_create(5);
    ERROR_IF(child_epollfd == -1, "child epoll_create");

    int connfd = the_users[idx].connfd;
    Addfd(child_epollfd, connfd);

    int pipefd = the_users[idx].pipefd[1];
    Addfd(child_epollfd, pipefd);

    AddSignal(SIGTERM, ChildTermHandler, false);

    while (!stop_child)
    {
        int number = epoll_wait(child_epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR))
        {
            printf("epoll failure");
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if ((sockfd == connfd) && (events[i].events & EPOLLIN))
            {
                memset(the_share_mem + idx * BUFFER_SIZE, '\0', BUFFER_SIZE);
                int ret = recv(connfd, the_share_mem + idx * BUFFER_SIZE, BUFFER_SIZE - 1, 0);
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        stop_child = true;
                    }
                }
                else if (ret == 0)
                {
                    stop_child = true;
                }
                else
                {
                    send(pipefd, (char*)&idx, sizeof(idx), 0);
                }
            }
            else if ((sockfd == pipefd) && (events[i].events & EPOLLIN))
            {
                int client = 0;
                int ret = recv(sockfd, (char*)&client, sizeof(client), 0);
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        stop_child = true;
                    }
                }
                else if (ret == 0)
                {
                    stop_child = true;
                }
                else
                {
                    send(connfd, the_share_mem + client * BUFFER_SIZE, BUFFER_SIZE, 0);
                }
            }
        }
    }
    close(connfd);
    close(pipefd);
    close(child_epollfd);
    return 0;
}

int main()
{
    Net::Log::SetLogger(Net::OUT_CONSOLE, Net::LOG_LEVEL_INFO);

    const int port = 8001;

    sockaddr_in addr{};
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int ret = bind(listenfd, (sockaddr*)&addr, sizeof(addr));
    ERROR_IF(ret == -1, "bind");

    ret = listen(listenfd, USER_LIMIT);

    users = new ClientData[USER_LIMIT + 1]{};
    pid_to_user_sub = new int[PROCESS_LIMIT];
    for (int i = 0; i < PROCESS_LIMIT; ++i)
    {
        pid_to_user_sub[i] = -1;
    }

    epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    Addfd(epollfd, listenfd);

    AddSignal(SIGCHLD, SignalHandler);
    AddSignal(SIGTERM, SignalHandler);
    AddSignal(SIGINT, SignalHandler);
    AddSignal(SIGPIPE, SignalHandler);

    shmfd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    ERROR_IF(shmfd == -1, "shm open");

    ret = ftruncate(shmfd, USER_LIMIT * BUFFER_SIZE);
    ERROR_IF(ret == -1, "ftruncate");

    share_mem = (char*)mmap(nullptr, USER_LIMIT * BUFFER_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    ERROR_IF(share_mem == MAP_FAILED, "share_mem");
    close(shmfd);

    bool stop_server = false;
    bool terminate = false;

    while (!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR))
        {
            LOG_WARN("epoll_wait failed");
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            if (sockfd == listenfd)
            {
                sockaddr_in client_addr{};
                socklen_t client_addrlength = sizeof(client_addr);
                int connfd = accept(listenfd, (sockaddr*)&client_addr, &client_addrlength);

                if (connfd < 0)
                {
                    LOG_WARN("accept failed");
                    continue;
                }

                if (user_count >= USER_LIMIT)
                {
                    const char *info = "too many users\n";
                    LOG_WARN("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }

                users[user_count].address = client_addr;
                users[user_count].connfd = connfd;
                ret = socketpair(PF_UNIX, SOCK_STREAM, 0, users[user_count].pipefd);
                ERROR_IF(ret == -1, "socket pair");
                pid_t pid = fork();
                if (pid < 0)
                {
                    close(connfd);
                    continue;
                }
                else if (pid == 0)
                {
                    close(epollfd);
                    close(listenfd);
                    close(users[user_count].pipefd[0]);
                    close(sig_pipe[0]);
                    close(sig_pipe[1]);

                    RunChild(user_count, users, share_mem);
                    munmap((void*)share_mem, USER_LIMIT * BUFFER_SIZE);
                    exit(0);
                }
                else
                {
                    close(connfd);
                    close(users[user_count].pipefd[1]);
                    Addfd(epollfd, users[user_count].pipefd[0]);
                    users[user_count].pid = pid;
                    pid_to_user_sub[pid] = user_count;
                    user_count++;
                }
            }
            else if ((sockfd == sig_pipe[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024]{};
                ret = recv(sockfd, signals, sizeof(signals), 0);
                if (ret <= 0)
                {
                    continue;
                }
                else
                {
                    for (int j = 0; j < ret; ++j)
                    {
                        switch (signals[i])
                        {
                            case SIGCHLD:
                            {
                                pid_t pid{};
                                int stat;
                                while ((pid == waitpid(-1, &stat, WNOHANG)) > 0)
                                {
                                    int del_user_sub = pid_to_user_sub[pid];
                                    pid_to_user_sub[pid] = -1;
                                    if ((del_user_sub < 0) || (del_user_sub > USER_LIMIT))
                                    {
                                        continue;
                                    }
                                    epoll_ctl(epollfd, EPOLL_CTL_DEL, users[del_user_sub].pipefd[0], 0);
                                    close(users[del_user_sub].pipefd[0]);

                                    users[del_user_sub] = users[--user_count];
                                    // 现在del_user_sub 是有效的 从尾部移动过来的
                                    pid_to_user_sub[users[del_user_sub].pid] = del_user_sub;
                                }
                                if (terminate && user_count == 0)
                                {
                                    stop_server = true;
                                }
                                break;
                            }
                            case SIGTERM:
                            case SIGINT:
                            {
                                LOG_WARN("kill al child now\n");
                                if (user_count == 0)
                                {
                                    stop_server = true;
                                    break;
                                }
                                for (int i1 = 0; i1 < user_count; ++i1)
                                {
                                    int pid = users[i].pid;
                                    kill(pid, SIGTERM);
                                }
                                terminate = true;
                                break;
                            }
                            default:
                                break;
                        }
                    }
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                int child = 0;
                ret = recv(sockfd, (char*)&child, sizeof(child), 0);
                LOG_INFO("read data from child accross pipe\n");
                if (ret <= 0)
                {
                    continue;
                }
                else
                {
                    for (int j = 0; j < user_count; ++j)
                    {
                        if (users[j].pipefd[0] != sockfd)
                        {
                            LOG_INFO("send data to child accross pie");
                            send(users[j].pipefd[0], (char*)&child, sizeof(child), 0);
                        }
                    }
                }
            }
        }
    }
}
