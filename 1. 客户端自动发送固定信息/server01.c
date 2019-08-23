#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
	
	assert(argc > 2);
	const char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	address.sin_family = AF_INET;

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock != -1);

	int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(sock, 5);
	assert(ret != -1);

	struct sockaddr_in client;
	socklen_t client_addrlength = sizeof(client);

	int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
	if(connfd < 0) {
		printf("error");
	} else {
		char buffer[BUF_SIZE];

		memset(buffer, '\0', BUF_SIZE);
		ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
		printf("got %d bytes of normal data '%s'\n", ret, buffer);

		memset(buffer, '\0', BUF_SIZE);
		ret = recv(connfd, buffer, BUF_SIZE - 1, MSG_OOB);
		printf("got %d bytes of obb data '%s'\n", ret, buffer);

		memset(buffer, '\0', BUF_SIZE);
		ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
		printf("got %d bytes of normal data '%s'\n", ret, buffer);
		

		close(connfd);
	}

	close(sock);
	return 0;
}
