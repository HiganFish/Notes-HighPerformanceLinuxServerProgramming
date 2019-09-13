#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

int setnonblocking(int fd) {
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epollfd, int fd, bool enable_et) {
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if(enable_et) {
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void lt(epoll_event* events, int number, int epollfd, int listenfd) {
	char buf[BUFFER_SIZE];
	for(int i = 0; i < number; i++) {
		int sockfd = events[i].data.fd;
		if(sockfd == listenfd) {
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, false);
		} else if(events[i].events & EPOLLIN) {
			printf("event trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
			if(ret <= 0) {
				close(sockfd);
				continue;
			}
			printf("get %dbytes of contents: %s\n", ret, buf);
		} else {
			printf("something else happened\n");
		}
	}
}

void et(epoll_event* events, int number, int epollfd, int listenfd) {
	char buf[BUFFER_SIZE];
	for(int i = 0; i < number; i++) {
		int sockfd = events[i].data.fd;
		if(sockfd == listenfd) {
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, true);

		} else if(events[i].events & EPOLLIN) {

			printf("event trigger once\n");
			while(1) {
				memset(buf, '\0', BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
				if(ret < 0) {
					if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
						printf("read later\n");
						break;
					}
					close(sockfd);
					break;
				} else if(ret == 0) {
					close(sockfd);
				} else {
					printf("get %dbytes of content: %s\n", ret, buf);
				}
			}
		} else {
			printf("something else happened\n");
		}
	}
}

int main(int argc, char* argv[]) {
	if(argc <= 2) {
		printf("Wrong number of parameters\n");
		return 1;
	}

	int port = atoi(argv[2]);
	char* ip = argv[1];

	struct sockaddr_in address;
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >=0);

	int ret = -1;
	while(ret == -1) {
		address.sin_port = htons(++port);
		ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
		printf("The port is %d\n", port);
	}

	ret = listen(listenfd, 5);
	assert(ret != -1);

	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd, true);
	while(1) {
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if(ret < 0) {
			printf("epool failure\n");
			break;
		}
		lt(events, ret, epollfd, listenfd);
		//et(events, ret, epollfd, listenfd);
	}
	close(listenfd);
	return 0;

}
