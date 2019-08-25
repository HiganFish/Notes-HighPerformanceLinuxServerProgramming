#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <curses.h>

#define BUFF_SIZE 1024

int main(int argc, char* argv[]) {

	if(argc <= 2) {
		printf("error, argue is %d\n", argc);
		return 1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	inet_pton(AF_INET, ip, &server_address.sin_addr);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);

	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	assert(sockfd >= 0);

	int conn = connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
	printf("error code is %d, error is %s\n", errno, strerror(errno));
	assert(conn != -1);

	char send_buffer[BUFF_SIZE];
	memset(send_buffer, '\0', BUFF_SIZE);
	
	char recv_buffer[BUFF_SIZE];

	socklen_t serveraddress_len = sizeof(server_address);
		
	while(1) {
		
		memset(recv_buffer, '\0', BUFF_SIZE);
		recvfrom(sockfd, recv_buffer, BUFF_SIZE - 1, MSG_DONTWAIT, (struct sockaddr*)&server_address, &serveraddress_len);
		if(recv_buffer[0] != '\0') {
			printf("msg: %s\n",recv_buffer);	
		}
		
		sleep(1);
		strcpy(send_buffer, "abv");
		sendto(sockfd, send_buffer, BUFF_SIZE - 1, 0,(struct sockaddr*)&server_address, sizeof(server_address));
		memset(send_buffer, '\0', BUFF_SIZE);
	}
	close(sockfd);
	return 0;
}

