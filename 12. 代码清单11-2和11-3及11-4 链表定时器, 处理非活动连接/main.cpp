//
// Created by lsmg on 2019/9/20.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <cerrno>
#include "lst_timer.h"
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define TIMESLOT 5

static int pipefd[2];
static sort_timer_lst timer_lst;
static int epollfd = 0;

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, old_option | O_NONBLOCK);
	return old_option;
}

void addfd(int epollfd, int fd)
{
	epoll_event event{};
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

// 信号集的处理函数， 发送信号到pipefd[1]， 然后从从pipefd[0]接受信息， 这样可以利用epoll来处理
void sig_handler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	send(pipefd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}

// 增加要处理的信号到信号集
void addsig(int sig)
{
	struct sigaction sa = {};
	sa.sa_handler = sig_handler; // 指定信号集的处理函数
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, nullptr) != -1);
}

void timer_handler()
{
	timer_lst.tick();
	alarm(TIMESLOT);
}

// 删除非活动连接socket上的注册时间， 并关闭
void cb_func(client_data* user_data)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
	assert(user_data);
	close(user_data->sockfd);
	printf("close fd %d\n", user_data->sockfd);
}

int main(int argc, char* argv[])
{

	if(argc <= 2)
	{
		printf("Wrong number of parameters\n");
		return 1;
	}

	const char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address = {};
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd, 5);
	assert(ret != -1);

	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd);

	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
	assert(ret != -1);
	setnonblocking(pipefd[1]);
	addfd(epollfd, pipefd[0]);

	addsig(SIGALRM);
	addsig(SIGTERM);
	bool stop_server = false;

	client_data* users = new client_data[FD_LIMIT];
	bool timeout = false;
	alarm(TIMESLOT);

	while (!stop_server)
	{
		/* 
		 * 这里还可以通过IO复用函数设置超时时间epoll_wait在时间超时后会返回0 进而处理相应信息
		 * 当然为了避免其他信息到来的影响 超时时间需要设置为变量， 保证每 一个设定时间 触发一次函数
		 * 
		 * 可以通过在epoll_wait设定一个start时间 在epoll_wait后设定end时间 新的等待时间减去时间差
		 * 如果减去后的时间小于0 则重置为 规定的正常时间间隔
		 * 
		 * */
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR))
		{
			printf("epoll failuer\n");
			break;
		}

		for (int i = 0; i < number; ++i)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == listenfd)
			{
				struct sockaddr_in client_address;
				socklen_t  client_addrlength = sizeof(client_address);
				int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
				addfd(epollfd, connfd);
				users[connfd].address = client_address;
				users[connfd].sockfd = connfd;
				
				// 创建定时器，设置回调函数与超时时间， 绑定定时器与用户数据
				util_timer* timer = new util_timer;
				timer->user_data = &users[connfd];
				timer->cb_func = cb_func;
				time_t cur = time(nullptr);
				
				// 设定 3  * TIMESLOT 超时时间, 但不是到时间就执行, 而是需要epoll_wait有了返回值才,并且每五秒一次的定时器sig_handler()被触发
				// 才能进入下方的 if判断设置 timeout为true, 进而执行尾部的timeout判断中的 timer_handler函数判断 timer_lst中的超时时钟 
				timer->expire = cur + 3  * TIMESLOT;
				users[connfd].timer = timer;
				timer_lst.add_timer(timer);
			}
			else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];
				ret = recv(pipefd[0], signals, sizeof(signals), 0);
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
					for (int i = 0; i < ret; ++i)
					{
						switch (signals[i])
						{
							case SIGALRM:
								// 标记有定时任务需要处理， 但不需要立即处理，可以先处理其他问题
								timeout = true;
								break;
							case SIGTERM:
								stop_server = true;
						}
					}
				}
			}
			
			// 处理客户端发来的信息
			else if (events[i].events & EPOLLIN)
			{
				memset(users[sockfd].buf, '\0', BUFFER_SIZE);
				ret = recv(sockfd, users[sockfd].buf, BUFFER_SIZE - 1, 0);
				printf("get %d bytes of client data %sfrom %d\n", ret, users[sockfd].buf, sockfd);
				util_timer* timer = users[sockfd].timer;
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
				// 对方关闭连接
				else if (ret == 0)
				{
					cb_func(&users[sockfd]);
					if (timer)
					{
						timer_lst.del_timer(timer);
					}
				}
				else
				{
					if (timer)
					{
						time_t cur = time(nullptr);
						timer->expire = cur + 3 * TIMESLOT;
						printf("adjust timer once\n");
						timer_lst.adjust_timer(timer);
					}
				}
				if (timeout)
				{
					timer_handler();
					timeout = false;
				}
			}
		}
	}
	close(listenfd);
	close(pipefd[1]);
	close(pipefd[0]);
	delete[] users;
	return 0;
}
