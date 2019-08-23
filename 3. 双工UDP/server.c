#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024

int main(int argc, char* argv[] ) {
	if(argc <= 2) {
		printf("arguement error\n");
		return 1;
	}

	char *ip = argv[1];
	int port = atoi(argv[2]);
	
	struct sockaddr_in address;
	memset(&address, '\0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);

	 int sock = socket(PF_INET, SOCK_DGRAM, 0);
	//int sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock >= 0);
	
	int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(sock, 5);
	
	struct sockaddr_in client;
	socklen_t client_addrlength = sizeof(client);

	//accept UDP²»ÐèÒªaccept
	//int conn = accept(sock, (struct sockaddr*)&client, &client_addrlength);
	//assert(conn >= -1);
	

	while(1) {
	
		char buff[1024];
		memset(buff, '\0', strlen(buff));
		recvfrom(sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr*)&client, &client_addrlength);
		printf("recv %s\n", buff);
		sendto(sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr*)&client, client_addrlength);
	}

	close(sock);
	return 0;
}
