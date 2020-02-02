#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include "lst_timer.cpp"
#include "util_lsmg.h"

static int pipefd[2];
static sort_timer_lst timer_lst;
static int epollfd = 0;

#define MAX_EVENT_NUMBER 1024
#define FD_LIMIT 65535
#define TIMESLOT 5

void setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig)
{
    struct sigaction sa{};
    sa.sa_flags |= SA_RESTART;
    sa.sa_handler = sig_handler;
    sigfillset(&sa.sa_mask);
    exit_if(sigaction(sig, &sa, nullptr) == -1, "add sig error");
}

void addfd(int fd)
{
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblock(fd);
}

void cb_func(client_data* user_data)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    close(user_data->sockfd);
    printf("close fd %d\n", user_data->sockfd);
}

void timer_handler()
{
    timer_lst.tick();
    alarm(TIMESLOT);
}
int main(int argc, char* argv[])
{
    exit_if(argc <= 2, "wrong number of parameters")

    char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    exit_if(listenfd == -1, "socket error")

    int ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
    exit_if(ret == -1, "bind error");

    ret = listen(listenfd, 5);
    exit_if(ret == -1, "listen errro");

    addsig(SIGALRM);
    addsig(SIGTERM);

    client_data *users = new client_data[FD_LIMIT];

    epoll_event events[MAX_EVENT_NUMBER];
    int epfd = epoll_create(5);
    epollfd = epfd;
    addfd(listenfd);

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    setnonblock(pipefd[1]);
    addfd(pipefd[0]);

    bool stop_server = false;
    bool timeout = false;
    alarm(TIMESLOT);

    while(!stop_server)
    {
        int number = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        exit_if((number < 0) && (errno != EINTR), "epoll_wait error");
        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_address{};
                socklen_t client_addresslen = sizeof(client_address);

                int newfd = accept(listenfd, (sockaddr*)&client_address, &client_addresslen);

                addfd(newfd);

                util_timer *timer = new util_timer;
                timer->user_data = &users[newfd];
                timer->cb_func = cb_func;
                time_t now = time(NULL);
                timer->expire = now + TIMESLOT * 3;

                users[newfd].address = client_address;
                users[newfd].sockfd = newfd;
                users[newfd].timer = timer;
                timer_lst.add_timer(timer);
            }
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(sockfd, signals, sizeof(signals), 0);
                if (ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int j = 0; j < ret; ++j)
                    {
                        switch(signals[j])
                        {
                            case SIGALRM:
                            {
                                timeout = true;
                                break;
                            }
                            case SIGTERM:
                            {
                                stop_server = true;
                            }
                        }
                    }
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                memset(users[sockfd].buf, '\0', BUFFER_SIZE);
                ret = recv(sockfd, users[sockfd].buf, BUFFER_SIZE - 1, 0);
                printf("get %d bytes of client data %s from %d\n", ret,
                        users[sockfd].buf, sockfd);

                util_timer *timer = users[sockfd].timer;
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        cb_func(&users[sockfd]);
                        if (timer)
                        {
                            timer_lst.del_timer(timer);
                        }
                    }
                }
                else if (ret == 0)
                {
                    cb_func(&users[sockfd]);
                    if (nullptr != timer)
                    {
                        timer_lst.del_timer(timer);
                    }
                }
                else
                {
                    if (nullptr !=timer)
                    {
                        time_t cur = time(NULL);
                        timer->expire = cur + 3 * TIMESLOT;
                        printf("adjust timr once\n");
                        timer_lst.adjust_timer(timer);
                    }
                }
            }
        }
        if (timeout)
        {
            timer_handler();
            timeout = false;
        }
    }
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete []users;
    return 0;
}