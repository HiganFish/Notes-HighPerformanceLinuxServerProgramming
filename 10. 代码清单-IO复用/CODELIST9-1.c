#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[] ) {
	
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

	struct sockaddr_in client_address;
	socklen_t client_addr_length = sizeof(client_address);
	int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addr_length);
	if(connfd <0) {
		printf("The error code is%d, error is%s\n", errno, strerror(errno));
		close(listenfd);
	}

	char buf[1024];
	fd_set read_fds, exception_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&exception_fds);

	while(1) {
	
		memset(buf, '\0', sizeof(buf));
		FD_SET(connfd, &read_fds);
		FD_SET(connfd, &exception_fds);
		ret = select(connfd + 1, &read_fds, NULL, &exception_fds, NULL);
		if(ret < 0) {
			printf("select error\n");
			break;
		}

		if(FD_ISSET(connfd, &read_fds)) {
			ret = recv(connfd, buf, sizeof(buf) - 1, 0);
			if(ret <= 0) {
				break;
			}

			printf("get %d bytes of normal data: %s\n", ret, buf);
		}

		if(FD_ISSET(connfd, &exception_fds)) {
			ret = recv(connfd, buf, sizeof(buf) - 1, 0);
			if(ret <= 0) {
				break;
			}

			printf("get %d bytes of obb data: %s\n", ret, buf);
		}
	}
	close(connfd);
	close(listenfd);
	return 0;

}
