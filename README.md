---
title: Linux高性能服务器编程记录

---
开头选了这本书, 看到这本书是他人总结三大本后自己写的, 先求个理解大概.
话说别人推荐的c++服务器咋都是c语言.....
2019年8月17日20:37:53

每学习一部分就写一个demo

# demo 记录

2019年8月20日 编写demo <1. 客户端自动发送固定信息> -- 所用基础连接部分和TCP连接部分

2019年8月22日 编写demo <2. 伪全双工TCP> -- 所用截止到网络Api之前

2019年8月22日 编写demo <3. 伪全双工UDP> -- 所用截止到网络Api之前

2019年8月25日 复现 源码 <4. 代码清单5-12 访问daytime服务> -- 第五章结束

## 客户端自动发送固定信息

## 伪全双工TCP

由于也算是初学C语言吧 中间出了好多问题 记录下来
```c
/*
*    以下这一部分 当初并不是这样写的final_msg在之间为 char *final_msg = "xxxx"; 这样在拼接字符串的时候
*    当然会报错, 因为这样的话在拼接的时候会超出长度溢出. 自己也用过几个语言了但是全有string代替 导致出现了这个问题
*/
char final_msg[100];
memset(final_msg, '\0', strlen(final_msg));
// from *.*.*.* : msg\n 
strcat(final_msg, "from ");
```
## 伪全双工UDP

accept UDP不需要accept
int conn = accept(sock, (struct sockaddr*)&client, &client_addrlength);
解释这个问题需要了解下 accept这个函数处于哪个阶段  下面这两张图片 完美的解释了这个问题

![](https://ftp.bmp.ovh/imgs/2019/08/e2abc0bdcc46c722.png)
![](https://ftp.bmp.ovh/imgs/2019/08/be70bff1d0f5524a.png)

与网络协议更加密切的见[博客](https://blog.csdn.net/yangbodong22011/article/details/69802544)
目前不太详细去了解这方面, 但是却是必要的
到现在@2019年8月23日21:22:52@ 为止 还是不明白我需要的是C++服务器编程, 但推荐却是C服务器编程的书籍

分了三篇
# 第一篇TCP/IP协议详解
## 第一章 TCP/IP协议族

### TCP/IP协议族体系结构和主要协议
协议族中协议众多, 这本书只选取了IP和TCP协议 - 对网络编程影响最直接

见得最多就是这四层结构了, 不过这本书写得更加详细一些
![](Linux高性能服务器编程读书记录/四层结构.jpg)

同样七层是osi参考模型, 简化后得到四层
![](https://gss1.bdstatic.com/9vo3dSag_xI4khGkpoWK1HF6hhy/baike/c0%3Dbaike80%2C5%2C5%2C80%2C26/sign=3a1768f4c6fc1e17e9b284632bf99d66/0dd7912397dda144d48ab350bbb7d0a20df48655.jpg)

不同层次之间, 通过接口互相交流, 这样方便了各层次的修改

#### 数据链路层
实现了网卡接口的网络驱动程序. 这里驱动程序方便了厂商的下层修改, 只需要向上层提供规定的接口即可.

存在两个协议 *ARP协议(Address Resolve Protocol, 地址解析协议)*. 还有*RARP(Reverse ~, 逆地址解析协议)*.  由于网络层使用IP地址寻址机器, 但是数据链路层使用物理地址(通常为MAC地址), 之间的转化涉及到ARP协议**ARP欺骗, 可能与这个有关, 目前不去学习**

#### 网络层
实现了数据包的选路和转发.  只有数据包到不了目标地址, 就`下一跳`(hop by hop), 选择最近的.
*IP协议(Internet Protocol)* 以及 *ICMP协议(Internet Control Message Protocol)* 
后者协议是IP协议的补充, 用来检测网络连接 1. 差错报文, 用来回应状态 2. 查询报文(ping程序就是使用的此报文来判断信息是否送达)

#### 传输层

为两台主机的应用提供端到端(end to end)的通信. 与网络层使用的下一跳不同, 他只关心起始和终止, 中转过程交给下层处理.
此层存在两大协议TCP协议和UDP协议

##### TCP协议

TCP协议(Transmission Control Protocol 传输控制协议) - 为应用层提供`可靠的, 面向连接, 基于流的服务`
通过`超时重传`和`数据确认`等确保数据正常送达.
TCP需要存储一些必要的状态, 可靠的协议

##### UDP协议
UPD协议(User Datagram Protocol 用户数据报协议) - 为应用层提供`不可靠的, 无连接的, 基于数据报的服务`
一般需要自己处理`数据确认`和`超时重传`的问题
通信两者不存储状态, 每次发送都需要指定地址信息. `有自己的长度`

#### 应用层

负责处理应用程序的逻辑

### 封装

上层协议发送到下层协议. 通过封装实现, 层与层之间传输的时候, 加上自己的头部信息.
被TCP封装的数据成为 `TCP报文段`
- 内核部分发送成功后删除数据

被UDP封装的数据成为 `UDP数据报`
- 发送后即删除

再经IP封装后成为`IP数据报`
最后经过数据链路层封装后为 `帧`


# 第二篇深入解析高性能服务器编程
## 第五章Linux网络编程基础API

socket基础api位于 `sys/socket.h` 头文件中
socket最开始的含义是 一个IP地址和端口对. 唯一的表示了TCP通信的一段
网络信息api `netdb.h`头文件中

### 主机字节序和网络字节序
字节序分为 `大端字节序`和`小端字节序`
由于大多数PC采用小端字节序(高位存在高地址处), 所以小端字节序又称为主机字节序

为了防止不同机器字节序不同导致的错乱问题. 规定传输的时候统一为 大端字节序(网络字节序).
这样主机会根据自己的情况决定 - 是否转换接收到的数据的字节序


### API

#### 基础连接
![](Linux高性能服务器编程读书记录/地址结构体.jpg)
![](Linux高性能服务器编程读书记录/协议组合地址族.jpg)
```c++
// 主机序和网络字节序转换
#include <netinet/in.h>
unsigned long int htonl (unsigned long int hostlong); // host to network long
unsigned short int htons (unsigned short int hostlong); // host to network short

unsigned long int htonl (unsigned long int netlong);
unsigned short int htons (unsigned short int netlong);

// IP地址转换函数
#include <arpa/inet.h>
// 将点分十进制字符串的IPv4地址, 转换为网络字节序整数表示的IPv4地址. 失败返回INADDR_NONE
in_addr_t  inet_addr( const char* strptr);

// 功能相同不过转换结果存在 inp指向的结构体中. 成功返回1 反之返回0
int inet_aton( const char* cp, struct in_addr* inp);

// 函数返回一个静态变量地址值, 所以多次调用会导致覆盖
char* inet_ntoa(struct in_addr in); 

// src为 点分十进制字符串的IPv4地址 或 十六进制字符串表示的IPv6地址 存入dst的内存中 af指定地址族
// 可以为 AF_INET AF_INET6 成功返回1 失败返回-1
int inet_pton(int af, const char * src, void* dst);
const char* inet_ntop(int af, const void*  src, char* dst, socklen_t cnt);


// 创建 命名 监听 socket
# include <sys/types.h>
# include <sys/socket.h>
// domain指定使用那个协议族 PF_INET PF_INET6
// type指定服务类型 SOCK_STREAM (TCP协议) SOCK_DGRAM(UDP协议)
// protocol设置为默认的0
// 成功返回socket文件描述符(linux一切皆文件), 失败返回-1
int socket(int domain, int type, int protocol);

// socket为socket文件描述符
// my_addr 为地址信息
// addrlen为socket地址长度
// 成功返回0 失败返回 -1
int bind(int socket, const struct sockaddr* my_addr, socklen_t addrlen);

// backlog表示队列最大的长度
int listen(int socket, int backlog);
// 接受连接 失败返回-1 成功时返回socket
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
```
客户端
```c
// 发起连接
#include <sys/types.h>
#include <sys/socket.h>
// 第三个参数为 地址指定的长度
// 成功返回0 失败返回-1
int connect(int sockfd, const struct sockaddr * serv_addr, socklen_t addrlen);

// 关闭连接
#include <unistd.h>
// 参数为保存的socket
// 并非立即关闭, 将socket的引用计数-1, 当fd的引用计数为0, 才能关闭(需要查阅)
int close(int fd);

// 立即关闭
#include <sys/socket.h>
// 第二个参数为可选值 
//	SHUT_RD 关闭读, socket的接收缓冲区的数据全部丢弃
//	SHUT_WR 关闭写 socket的发送缓冲区全部在关闭前发送出去
//	SHUT_RDWR 同时关闭读和写
// 成功返回0 失败为-1 设置errno
int shutdown(int sockfd, int howto)
```
#### 基础TCP
```c
#include<sys/socket.h>
#include<sys/types.h>

// 读取sockfd的数据
// buf 指定读缓冲区的位置
// len 指定读缓冲区的大小
// flags 参数较多
// 成功的时候返回读取到的长度, 可能小于预期长度, 需要多次读取.   读取到0 通信对方已经关闭连接, 错误返回-1
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
// 发送
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```

| 选项名        | 含义                                                                                     | 可用于发送 | 可用于接收 |
| ------------- | ---------------------------------------------------------------------------------------- | ---------- | ---------- |
| MSG_CONFIRM   | 指示链路层协议持续监听, 直到得到答复.(仅能用于SOCK_DGRAM和SOCK_RAW类型的socket)          | Y          | N          |
| MSG_DONTROUTE | 不查看路由表, 直接将数据发送给本地的局域网络的主机(代表发送者知道目标主机就在本地网络中) | Y          | N          |
| MSG_DONTWAIT  | 非阻塞                                                                                   | Y          | Y          |
| MSG_MORE      | 告知内核有更多的数据要发送, 等到数据写入缓冲区完毕后,一并发送.减少短小的报文提高传输效率 | Y          | N          |
| MSG_WAITALL   | 读操作一直等待到读取到指定字节后才会返回                                                 | N          | Y          |
| MSG_PEEK      | 看一下内缓存数据, 并不会影响数据                                                         | N          | Y          |
| MSG_OOB       | 发送或接收紧急数据                                                                       | Y          | Y          |
| MSG_NOSIGNAL  | 向读关闭的管道或者socket连接中写入数据不会触发SIGPIPE信号                                | Y          |        N    |

#### 基础UDP
```c
#include <sys/types.h>
#include <sys/socket.h>
// 由于UDP不保存状态, 每次发送数据都需要 加入目标地址.
// 不过recvfrom和sendto 也可以用于 面向STREAM的连接, 这样可以省略发送和接收端的socket地址
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
ssize_t sendto(int sockfd, const void* buf, size_t len, ing flags, const struct sockaddr* dest_addr, socklen_t addrlen);

```
#### 通用读写函数
```c
#inclued <sys/socket.h>
ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags);
ssize_t sendmsg(int sockfd, struct msghdr* msg, int flags);

struct msghdr {
/* socket address --- 指向socket地址结构变量, 对于TCP连接需要设置为NULL*/
	void* msg_name; 


	socklen_t msg_namelen;
	
	/* 分散的内存块 --- 对于 recvmsg来说数据被读取后将存放在这里的块内存中, 内存的位置和长度由
     * msg_iov指向的数组指定, 称为分散读(scatter read)  ---对于sendmsg而言, msg_iovlen块的分散内存中
     * 的数据将一并发送称为集中写(gather write);
	*/
	struct iovec* msg_iov; 
	int msg_iovlen; /* 分散内存块的数量*/
	void* msg_control; /* 指向辅助数据的起始位置*/
	socklen_t msg_controllen; /* 辅助数据的大小*/
	int msg_flags; /* 复制函数的flags参数, 并在调用过程中更新*
};

struct iovlen {
	void* iov_base /* 内存起始地址*/
	size_t iov_len /* 这块内存长度*/
	
}
```
#### 其他Api
```c
#include <sys/socket.h>
// 用于判断 sockfd是否处于带外标记, 即下一个被读取到的数据是否是带外数据, 
// 是的话返回1, 不是返回0
// 这样就可以选择带MSG_OOB标志的recv调用来接收带外数据. 
int sockatmark(int sockfd);

// getsockname 获取sockfd对应的本端socket地址, 存入address指定的内存中, 长度存入address_len中 成功返回0失败返回-1
// getpeername 获取远端的信息, 同上
int getsockname(int sockfd, struct sockaddr* address, socklen_t* address_len);
int getpeername(int sockfd, struct sockaddr* address, socklen_t* address_len);

/* 以下函数头文件均相同*/

// sockfd 目标socket, level执行操作协议(IPv4, IPv6, TCP) option_name 参数指定了选项的名字. 后面值和长度
// 成功时返回0 失败返回-1
int getsockopt(int sockfd, int level, int option_name, void* option_value, 
						socklen_t restrict option_len);
int setsockopt(int sockfd, int level, int option_name, void* option_value, 
						socklen_t restrict option_len);
```

| SO_REUSEADDR | 重用本地地址      | sock被设置此属性后, 即使sock在被bind()后处于TIME_WAIT状态, 此时与他绑定的socket地址依然能够立即重用来绑定新的sock        |
| ------------ | ----------------- | ------------------------------------------------------------------------------------------------------------------------ |
| SO_RCVBUF    | TCP接收缓冲区大小 | 最小值为256字节. 设置完后系统会自动加倍你所设定的值. 多出来的一倍将用用作空闲缓冲区处理拥塞                              |
| SO_SNDBUF    | TCP发送缓冲区大小 | 最小值为2048字节                                                                                                         |
| SO_RCVLOWAT  | 接收的低水位标记  | 默认为1字节, 当TCP接收缓冲区中可读数据的总数大于其低水位标记时, IO复用系统调用将通知应用程序可以从对应的socket上读取数据 |
| SO_SNDLOWAT  | 发送的高水位标记  | 默认为1字节, 当TCP发送缓冲区中空闲空间大于低水位标记的时候可以写入数据                                                   |
| SO_LINGER    |                   |                                                                                                                          |


```c
struct linger {
	int l_onoff /* 开启非0, 关闭为0*/
	int l_linger; /* 滞留时间*/
	/*
	* 当onoff为0的时候此项不起作用, close调用默认行为关闭socket
	* 当onoff不为0 且linger为0, close将立即返回, TCP将丢弃发送缓冲区的残留数据, 同时发送一个复位报文段
	* 当onoff不为0 且linger大于0 . 当socket阻塞的时候close将会等待TCP模块发送完残留数据并得到确认后关 
	* 闭, 如果是处于非阻塞则立即关闭
	*/
};
```
#### 网络信息API
```c
#include <netdb.h>
// 通过主机名查找ip
struct hostent* gethostbyname(const char* name);

// 通过ip获取主机完整信息 
// type为IP地址类型 AF_INET和AF_INET6
struct hostent* gethostbyaddr(const void* addr, size_t len, int type);

struct hostent {
	char* h_name;
	char ** h_aliases;
	int h_addrtype;
	int h_length;
	char** h_addr_list; /* 按网络字节序列出主机IP地址列表*/
}
```
以下两个函数通过读取/etc/services文件 来获取服务信息
```c
#include <netdb.h>
// 根据名称获取某个服务的完整信息
struct servent getservbyname(const char* name, const char* proto);

// 根据端口号获取服务信息
struct servent getservbyport(int port, const char* proto);

struct servent {
	char* s_name; /* 服务名称*/
	char ** s_aliases; /* 服务的别名列表*/
	int s_port; /* 端口号*/
	char* s_proto; /* 服务类型, 通常为TCP或UDP*/
}
```
```c
#include <netdb.h>
// 内部使用的gethostbyname 和 getserverbyname
// hostname 用于接收主机名, 也可以用来接收字符串表示的IP地址(点分十进制, 十六进制字符串)
// service 用于接收服务名, 字符串表示的十进制端口号
// hints参数 对getaddrinfo的输出进行更准确的控制, 可以设置为NULL, 允许反馈各种有用的结果
// result 指向一个链表, 用于存储getaddrinfo的反馈结果
int getaddrinfo(const char* hostname, const char* service, const struct addrinfo* hints, struct addrinfo** result)

struct addrinfo{
	int ai_flags;
	int ai_family;
	int ai_socktype; /* 服务类型, SOCK_STREAM或者SOCK_DGRAM*/
	int ai_protocol;
	socklen_t ai_addrlen;
	char* ai_canonname; /* 主机的别名*/
	struct sockaddr* ai_addr; /* 指向socket地址*/
	struct addrinfo* ai_next; /* 指向下一个结构体*/
}

// 需要手动的释放堆内存
void freeaddrinfo(struct addrinfo* res);
```
![](https://ftp.bmp.ovh/imgs/2019/08/7ebedb14d8eedeac.png)

```c
#include <netdb.h>
// host 存储返回的主机名
// serv存储返回的服务名

int getnameinfo(const struct sockaddr* sockaddr, socklen_t addrlen, char* host, socklen_t hostlen, char* serv
	socklen_t servlen, int flags);

```
![](https://ftp.bmp.ovh/imgs/2019/08/bc7196e9a30d5152.png)

测试
使用
```shell
telnet ip port #来连接服务器的此端口
netstat -nt | grep port #来查看此端口的监听
```

