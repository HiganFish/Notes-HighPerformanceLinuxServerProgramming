#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char* argv[]) {
	
	// 检测参数个数 
	if(argc != 2) {
		printf("Wrong number of parameters.\n");
		return 1;
	}
	
	char* hostname = argv[1];
	
	// 通过主机名获取主机信息 
	struct hostent* hostinfo = gethostbyname(hostname);
	assert(hostinfo);
	
	// 通过服务名搜索服务 
	struct servent* servinfo = getservbyname("daytime", "tcp");
	assert(servinfo);
	printf("the daytime port is %d\n", ntohs(servinfo -> s_port));
	
	// 设定地址信息 
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = servinfo -> s_port;
	address.sin_addr = *(struct in_addr*)* hostinfo -> h_addr_list;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int result = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
	assert(result != -1);

	char buffer[128];
	result = read(sockfd, buffer, sizeof(buffer));
	assert(result > 0);
	buffer[result] = '\0';

	printf("the day time is: %s\n", buffer);
	close(sockfd);
	return 0;
}
