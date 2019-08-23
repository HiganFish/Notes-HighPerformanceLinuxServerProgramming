#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#define BUF_SIZE 1024


// 将收到的信息分发, 提供链接数组 总连接数和发信链接标号 
void distributeMessage(int* conns, int connAmount, int nowConn,char* clientIp, char* msg) {
    char final_msg[100];
    memset(final_msg, '\0', strlen(final_msg));
	
	// from *.*.*.* : msg\n 
    strcat(final_msg, "from ");
    strcat(final_msg, clientIp);
    strcat(final_msg, ": ");
    strcat(final_msg, msg);
    strcat(final_msg, "\n");
    
    for(int i =0; i <= connAmount && i != nowConn; i++) {
        send(conns[i], final_msg, BUF_SIZE - 1, 0);
    }

}


int main(int argc, char* argv[]) {

    if(argc <= 2) {
        printf("error, arrgument amount error");
        return 1;
    }

    char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    address.sin_family = AF_INET;

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int resue = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &resue, sizeof(resue));

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    if(ret == -1) {
        printf("error code is: %d, error is: %s\n", errno, strerror(errno));
        return 1;
    }

    ret = listen(sock, 5);
    assert(ret != -1);

    int maxClient = 2;
    int clientAmount = 0;
    struct sockaddr_in clients[maxClient];

    int acceptSocket[maxClient];
    int acceptConn[maxClient];

    while(clientAmount != maxClient) {

        struct sockaddr_in* client = &clients[clientAmount];

        socklen_t client_addrlength = sizeof(client);
        int conn = accept(sock, (struct sockaddr*)client, &client_addrlength);
        if(conn != -1) {
            char newSocketIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client -> sin_addr), newSocketIp, INET_ADDRSTRLEN);
            printf("新连接建立 - %d, ip地址为 - %s\n", sock, newSocketIp);
            acceptSocket[clientAmount] = sock;
            acceptConn[clientAmount] = conn;
            clientAmount++;
        }
    }

    for(int i = 0; ; i++) {
        char recvBuff[BUF_SIZE];
        memset(recvBuff, '\0', BUF_SIZE);
        recv(acceptConn[i%maxClient], recvBuff, BUF_SIZE - 1, MSG_DONTWAIT);

        if(recvBuff[0] != '\0') {

            char newMsgIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clients[i%maxClient].sin_addr), newMsgIp, INET_ADDRSTRLEN);
            printf("recv msg from %s: %s\n", newMsgIp, recvBuff);

            distributeMessage(acceptConn, clientAmount - 1, i%maxClient - 1, newMsgIp, recvBuff);
        }
        if(!strcmp(recvBuff, "exit")) {
            return 1;
        }
    }
    return 0;
}
