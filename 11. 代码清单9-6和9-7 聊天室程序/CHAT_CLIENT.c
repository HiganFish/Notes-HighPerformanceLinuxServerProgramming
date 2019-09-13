#include <sys/socket.h>
#include <sys/types.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#define BUFFER_SIZE 64

int main(int argc, char* argv[]) {

	if(argc <= 2) {
		printf("Wrong number of parameters \n");
		return 1;
	}

	int port = atoi(argv[2]);
	char* ip = argv[1];

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);

	if(connect(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		printf("Connect error\n");
		close(sockfd);
		return 1;
	}

	pollfd fds[2];
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	fds[1].fd = sockfd;
	fds[1].events = POLLIN | POLLRDHUP;
	fds[1].revents = 0;

	char read_buf[BUFFER_SIZE];
	int pipefd[2];
	int ret = pipe(pipefd);
	assert(ret != -1);

	while (1) {

		ret = poll(fds, 2, -1);
		if(ret < 0) {
			printf("poll failure\n");
			break;
		}

		if(fds[1].revents & POLLRDHUP) {
			printf("server close the connection\n");
			break;
		}
		if(fds[1].revents & POLLIN) {
			memset(read_buf, '\0', BUFFER_SIZE);
			recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
			printf("%s\n", read_buf);
		}

		if(fds[0].revents & POLLIN) {
			ret = splice(0, nullptr, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
			ret = splice(pipefd[0], nullptr, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
		}
	}

	close(sockfd);
	return 0;
}
