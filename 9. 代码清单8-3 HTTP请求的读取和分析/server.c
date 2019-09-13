#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

#define BUFFER_SIZE 4096

/* 主状态机的两种状态*/ 
enum CHECK_STATE {CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER};

/* 从状态机的三种状态, 读取到完整行, 行出错, 行不完整需要再次读取*/
enum LINE_STATUS {LINE_OK = 0, LINE_BAD, LINE_OPEN};

/* 处理HTTP的结果 NO_REQUEST-不完整, GET_REQUEST-完整请求, BAD_REQUEST-错误请求, 无权限, 内部错误, 客户端连接关闭*/
enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};

static const char* szret[] = {"I get a correct result\n", "Something wrong\n"};

/* 从状态机, 解析出一行内容*/
LINE_STATUS parse_line(char* buffer, int& checked_index, int& read_index) {
    char temp;
	
	// 分析buffer中是否有\r字符, 只有\r字符说明不完整, \r\n都存在说明完整, 都不存在说明不完整 
    for(; checked_index < read_index; checked_index++) {
        temp = buffer[checked_index];
        if(temp == '\r') {
            if((checked_index + 1) == read_index) {
                return LINE_OPEN;
            } else if(buffer[checked_index + 1] == '\n') {

                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }

            return LINE_BAD;
        } else if(temp == '\n') {

            if((checked_index > 1) && buffer[checked_index - 1] == '\r') {

                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

HTTP_CODE parse_requestline(char* temp, CHECK_STATE& checkstate) {
	// Http请求必定带有空白字符 或者 \t字符 
    char* url = strpbrk(temp, " \t");
    if(!url) {
        return BAD_REQUEST;
    }
    *url++ = '\0';

    char* method = temp;
    // 仅支持GET方法 
    if(strcasecmp(method, "GET") == 0) {
        printf("The request method is GET\n");
    } else {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    char* version = strpbrk(url, " \t");
    if(!version) {
        return BAD_REQUEST;
    }

    *version++ = '\0';
    version += strspn(version, " \t");
    // 仅支持HTTP1.1 
    if(strcasecmp(version, "HTTP/1.1") != 0) {
        return BAD_REQUEST;
    }
    // 检查URL完整性 
    if(strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    }
    if(!url || url[0] != '/') {
        return BAD_REQUEST;
    }
    printf("The request URL is: %s\n", url);

    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

// 处理头部信息 
HTTP_CODE parse_headers(char* temp) {
    if(temp[0] == '\0') {
        return GET_REQUEST;
    } else if (strncasecmp(temp, "Host:", 5) == 0) {
        temp += 5;
        temp += strspn(temp, "\ t");
        printf("The request host is: %s\n", temp);
    } else {
        printf("I can not handle this header\n");
    }
    return NO_REQUEST;
}

HTTP_CODE parse_content(char* buffer, int& checked_index, CHECK_STATE&
                        checkstate, int& read_index, int& start_line) {
	LINE_STATUS linestatus = LINE_OK;
	HTTP_CODE retcode = NO_REQUEST;

	while((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK) {

		char* temp = buffer + start_line;
		start_line = checked_index;

		switch(checkstate) {
			case CHECK_STATE_REQUESTLINE:
				retcode = parse_requestline(temp, checkstate);
				if(retcode == BAD_REQUEST) {
					return BAD_REQUEST;
				}
				break;
			case CHECK_STATE_HEADER:
				retcode = parse_headers(temp);
				if(retcode == BAD_REQUEST) {
					return BAD_REQUEST;
				} else if (retcode == GET_REQUEST) {
					return GET_REQUEST;
				}
				break;
			default:
				return INTERNAL_ERROR;
		}
	}
	if(linestatus == LINE_OPEN) {
		return NO_REQUEST;
	} else {
		return BAD_REQUEST;
	}
}

int main(int argc, char* argv[]) {
	if(argc <= 2) {
		printf("Wrong number of parameters\n");
		return 1;
	}

	const char * ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));

	//assert(ret != -1);
	while (ret == -1) {
		address.sin_port = htons(++port);
		ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
		printf("the port is %d\n", port);
	}

	ret = listen(listenfd, 5);
	assert(ret != -1);
	struct sockaddr_in client_address;
	socklen_t client_addrlength = sizeof(client_address);
	int fd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);

	if(fd < 0) {
		printf("errno is: %d\n", errno);
	} else {
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', BUFFER_SIZE);
		int date_read = 0;
		int read_index = 0;
		int checked_index = 0;
		int start_line = 0;

		CHECK_STATE checkstatus = CHECK_STATE_REQUESTLINE;
		while(1) {
			date_read = recv(fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
			if(date_read == -1) {
				printf("reading failed\n;");
				break;
			} else if(date_read == 0) {
				printf("remote client had closed the connection\n");
				break;
			}
			read_index += date_read;

			HTTP_CODE result = parse_content(buffer, checked_index, checkstatus, read_index, start_line);
			if(result == NO_REQUEST) {
				continue;
			} else if(result == GET_REQUEST) {
				send(fd, szret[0], strlen(szret[0]), 0);
				break;
			} else {
				send(fd, szret[1], strlen(szret[1]), 0);
				break;
			}
		}
		close(fd);
	}
	close(listenfd);
	return 0;
}
