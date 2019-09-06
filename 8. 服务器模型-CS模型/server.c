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

#define BUFFER_SIZE 1024
void distributeMessage(int sock_client, char* msg);
int accountProcess(char* msg, int msg_socket);
void processMessage(char* msg, int msg_socket);

int accountLogin(char* username, char* password, int msg_socket);
void accountCreate(char* username, char* password);

struct user {
    char nikename[10];
    char username[10];
    char password[16];
    int status;
};

struct online_user {
    char nikename[10];
    int sockfd;
    int status;
};

struct user users[100];
struct online_user online_users[100];
int sum_user = -1;
int sum_online_user = -1;

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

    fd_set readfds, testfds;

    FD_ZERO(&readfds);
    FD_SET(sock_server, &readfds);

    while(1) {

        testfds = readfds;
        int result_select = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval*)0);
        if(result_select < 1) {
            perror("server error\n");
            exit(1);
        }

        int count_fd;
        for(count_fd = 0; count_fd < FD_SETSIZE; count_fd++) {
            if(FD_ISSET(count_fd, &testfds)) {

                if(count_fd == sock_server) {
                    int len_client = sizeof(address_client);
                    int sock_client = accept(sock_server, (struct sockaddr*)&address_client, &len_client);
                    FD_SET(sock_client, &readfds);
                    printf("add a new client to readfds %d\n", sock_client);
                } else {

                    int pipefd[2];
                    int result_pipe = pipe(pipefd);

                    char msg[BUFFER_SIZE];
                    memset(msg, '\0', BUFFER_SIZE);
                    result_pipe = splice(count_fd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE | SPLICE_F_NONBLOCK);

                    if(result_pipe == -1) {
                        close(count_fd);
						// 客户端掉线后清除客户端 
                        for(int i =0; i<= sum_online_user; i++) {
                            
                            if(online_users[i].sockfd == i) {
                                online_users[i].status = 0;
                            }
                        }
                    }

                    read(pipefd[0], msg, BUFFER_SIZE - 1);

                    processMessage(msg, count_fd);
                }
            }
        }
    }
    printf("close socket\n");
    close(sock_server);
    return 0;
}

#define LOGIN_FLAG 1
#define SENDMSG_FLAG 2

#define LOGIN_SUCCESS 1
#define LOGIN_PASSWORD_WRONG 2
#define LOGIN_SUCCESS_NEW_USER 3
#define LOGIN_ERROR_LIGINED 4
void processMessage(char* msg, int msg_socket) {
    if(msg[0] == '{' && msg[2] == '@' && msg[strlen(msg) - 3] == '}') {

        if(msg[1] ==  48 + LOGIN_FLAG) { // 请求为登陆
            
            int result = accountProcess(msg + 3, msg_socket);

            char resu[1024];
            memset(resu, '\0', 1024);

            if(result == LOGIN_SUCCESS) {
                strcat(resu, "{6@LOGIN_SUCCESS}\r\n");
            } else if(result == LOGIN_SUCCESS_NEW_USER) {
                strcat(resu, "{6@LOGIN_SUCCESS_NEW_USER PLEASE LOGIN IN}\r\n");
            } else if(result == LOGIN_PASSWORD_WRONG) {
                strcat(resu, "{6@LOGIN_PASSWORD_WRONG}\r\n");
            } else if(result == LOGIN_ERROR_LIGINED) {
                strcat(resu, "{6@DON`t LOGIN AGAIN}\r\n");
            }

            send(msg_socket, resu, strlen(resu), 0);
            
        } else if(msg[1] == 48 + SENDMSG_FLAG){ // 请求为发送信息
            distributeMessage(msg_socket, msg);
        }
    }
}

// 从msg中获取账号密码 并分发到登陆或者注册
int accountProcess(char* msg, int msg_socket) {
    char username[10];

    char password[16];
    int account_length = 0;
    int is_username = 1;
    for(int i = 0; i < strlen(msg) - 3; i++) {
        if(is_username) {

            if(msg[i] == '@') {
                is_username = 0;
                account_length = 0;
                continue;
            }

            username[account_length++] = msg[i];

        } else {
            password[account_length++] = msg[i];
        }
    }

    int login_result = accountLogin(username, password, msg_socket);
    if(login_result != 0) {
        return login_result;
    }

    accountCreate(username, password);

    return LOGIN_SUCCESS_NEW_USER;
}

int accountLogin(char* username, char* password, int msg_socket) {
    for(int i = 0; i <= sum_user; i++) {

        if(!strcmp(users[i].username, username)) {
            if(!strcmp(users[i].password, password)) {
                // login success

                for(int relogin_test_count = 0; relogin_test_count <= sum_online_user; relogin_test_count++) {

                    if(online_users[relogin_test_count].sockfd == msg_socket) {
                        return LOGIN_ERROR_LIGINED;
                    }
                }

                sum_online_user++;
                strcpy(online_users[sum_online_user].nikename, users[i].nikename);
                online_users[sum_online_user].status = 1;
                online_users[sum_online_user].sockfd = msg_socket;
                return LOGIN_SUCCESS;
            } else {
                return LOGIN_PASSWORD_WRONG;
            }
        }
    }

    return 0;
}

void accountCreate(char* username, char* password) {
    sum_user++;

    struct user new_user;
    memset(&new_user, 0, sizeof(new_user));
    strcpy(new_user.username, username);
    strcpy(new_user.password, password);

    char nikename[10];
    memset(nikename, '\0', 10);
    strcat(nikename, "HJKSDK");
    nikename[strlen(nikename)] = 48 + sum_user;
    strcpy(new_user.nikename, nikename);

    users[sum_user] = new_user;
}

void distributeMessage(int sock_client, char* msg) {
    char* recvMsg = "{6@ok}\r\n";

    for(int i = 0; i <= sum_online_user; i++) {
        struct online_user the_user = online_users[i];
        if(the_user.sockfd != sock_client && the_user.status == 1) {
            char ret_msg[1024];
            memset(ret_msg, '\0', 1024);
            strcat(ret_msg, "from ");
            strcat(ret_msg, the_user.nikename);
            strcat(ret_msg, " :");
            strcat(ret_msg, msg);

            send(the_user.sockfd, ret_msg, strlen(ret_msg), 0);
        }
    }

    send(sock_client, recvMsg, strlen(recvMsg), 0);
}
