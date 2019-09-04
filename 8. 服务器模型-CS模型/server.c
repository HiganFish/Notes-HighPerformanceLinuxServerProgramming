// 如果不加入 _GNU_SOURCE符号常亮 fcntl头文件会报错 
#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {

	if(argc <= 2) {
		printf("Wrong number of parameters!\n");
		return 1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);
	struct sockaddr_in address_server, address_client;
	memset(&address_server, 0, sizeof(address_server));
	address_server.sin_family = AF_INET;
	address_server.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address_server.sin_addr);

	int sock_server = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock_server >= 0);
	int ret = bind(sock_server, (struct sockaddr*)&address_server, sizeof(address_server));
	assert(ret != -1);
	ret = listen(sock_server, 5);
	assert(ret != -1);
	
	// readfds 负责存储客户端  testfds 负责存储变动的文件描述符 
	fd_set readfds, testfds;

	FD_ZERO(&readfds);
	FD_SET(sock_server, &readfds);

	while(1) {
		
		testfds = readfds;
		// 获取变动的文件描述符 写操作 
		int result_select = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval*)0);
		if(result_select < 1) {
			perror("server error\n");
			exit(1);
		}
		
		int count_fd; 
		for(count_fd = 0; count_fd < FD_SETSIZE; count_fd++) {
			
			if(FD_ISSET(count_fd, &testfds)) {
				
				// 如果为 服务器fd 则接受新的连接并放入 readfds
				if(count_fd == sock_server) {
					int len_client = sizeof(address_client);
					int sock_client = accept(sock_server, (struct sockaddr*)&address_client, &len_client);
					FD_SET(sock_client, &readfds);
					printf("add a new client to readfds %d\n", sock_client);
				} else {
				
					// 发回客户端发送的信息 
					int pipefd[2];
					int result_pipe = pipe(pipefd);
					// 设置SPLICE_F_NONBLOCK 非堵塞防止客户端掉线的等待 
					result_pipe = splice(count_fd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
					// 客户端掉线后清除客户端 
					if(result_pipe == -1) {
						close(count_fd);
						FD_CLR(count_fd, &readfds);
					}
					
					result_pipe = splice(pipefd[0], NULL, count_fd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
				}
			}
		}
	}
}
