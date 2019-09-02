#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
	
	if(argc <= 2) {
		printf("Wrong number of parameters!\n");
		return 1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);

	// construct socket address
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip,&address.sin_addr);

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock >= 0);

	int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(sock, 5);
	assert(ret != -1);

	struct sockaddr_in client;
	socklen_t client_length = sizeof(client);
	
	while(1) {
	
		int conn = accept(sock, (struct sockaddr*)&client, &client_length);
		
		char ip_client[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client.sin_addr, ip_client, INET_ADDRSTRLEN);
		int port_client;
		port_client = ntohs(client.sin_port);
		printf("A new client connect ip-%s, port-%d\n", ip_client, port_client);

		char buffer[BUFFER_SIZE];
		while(1) {
			memset(buffer, '\0', BUFFER_SIZE);
			recv(conn, buffer, BUFFER_SIZE - 1, 0);
			if(buffer[0] == '\0') {
				printf("client is dead!\n");
				break;
			}
			printf("recv %s",buffer);
		}
	}
	close(sock);
	return 0;
}
