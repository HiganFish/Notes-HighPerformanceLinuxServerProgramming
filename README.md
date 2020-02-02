# 第一篇TCP/IP协议详解
## 第一章 TCP/IP协议族

### TCP/IP协议族体系结构和主要协议
协议族中协议众多, 这本书只选取了IP和TCP协议 - 对网络编程影响最直接

见得最多就是这四层结构了, 不过这本书写得更加详细一些
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%9B%9B%E5%B1%82%E7%BB%93%E6%9E%84.jpg)

同样七层是osi参考模型, 简化后得到四层
![](https://gss1.bdstatic.com/9vo3dSag_xI4khGkpoWK1HF6hhy/baike/c0%3Dbaike80%2C5%2C5%2C80%2C26/sign=3a1768f4c6fc1e17e9b284632bf99d66/0dd7912397dda144d48ab350bbb7d0a20df48655.jpg)
不同层次之间, 通过接口互相交流, 这样方便了各层次的修改

**应用层**
负责处理应用程序的逻辑

**表示层**
定义了数据的格式及加密

**会话层**
它定义了如何开始、控制和结束一个会话，包括对多个双向消息的控制和管理，以便在只完成连续消息的一部分时可以通知应用，从而使表示层看到的数据是连续的

**传输层**
为两台主机的应用提供端到端(end to end)的通信. 与网络层使用的下一跳不同, 他只关心起始和终止, 中转过程交给下层处理.
此层存在两大协议TCP协议和UDP协议
*TCP协议*
TCP协议(Transmission Control Protocol 传输控制协议) - 为应用层提供`可靠的, 面向连接, 基于流的服`
通过`超时重传`和`数据确认`等确保数据正常送达.
TCP需要存储一些必要的状态, 可靠的协议
*UDP协议*
UPD协议(User Datagram Protocol 用户数据报协议) - 为应用层提供`不可靠的, 无连接的, 基于数据报的服务`
一般需要自己处理`数据确认`和`超时重传`的问题
通信两者不存储状态, 每次发送都需要指定地址信息. `有自己的长度`

**网络层**
实现了数据包的选路和转发.  只有数据包到不了目标地址, 就`下一跳`(hop by hop), 选择最近的.
*IP协议(Internet Protocol)* 以及 *ICMP协议(Internet Control Message Protocol)* 
后者协议是IP协议的补充, 用来检测网络连接 1. 差错报文, 用来回应状态 2. 查询报文(ping程序就是使用的此报文来判断信息是否送达)

**数据链路层**
实现了网卡接口的网络驱动程序. 这里驱动程序方便了厂商的下层修改, 只需要向上层提供规定的接口即可.
存在两个协议 *ARP协议(Address Resolve Protocol, 地址解析协议)*. 还有*RARP(Reverse ~, 逆地址解析协议)*.  由于网络层使用IP地址寻址机器, 但是数据链路层使用物理地址(通常为MAC地址), 之间的转化涉及到ARP协议**ARP欺骗, 可能与这个有关, 目前不去学习**

**封装**
上层协议发送到下层协议. 通过封装实现, 层与层之间传输的时候, 加上自己的头部信息.
被TCP封装的数据成为 `TCP报文段`
- 内核部分发送成功后删除数据

被UDP封装的数据成为 `UDP数据报`
- 发送后即删除

再经IP封装后成为`IP数据报`
最后经过数据链路层封装后为 `帧`

下面的操作都将在如下环境进行
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E6%B5%8B%E8%AF%95%E7%BD%91%E7%BB%9C.png)

**ARP**
ARP协议能实现任意网络层地址到任意物理地址的转换

## 第二章 IP协议详解
IP协议是TCP/IP协议簇的核心协议, 是socket网络编程的基础之一
IP协议为上层协议提供无状态, 无连接, 不可靠的服务


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

**基础连接**
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%9C%B0%E5%9D%80%E7%BB%93%E6%9E%84%E4%BD%93.jpg)
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%8D%8F%E8%AE%AE%E7%BB%84%E5%90%88%E5%9C%B0%E5%9D%80%E6%97%8F.jpg)
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
// 协议名, 需要转换的ip, 存储地址, 长度(有两个常量 INET_ADDRSTRLEN, INET6_ADDRSTRLEN)
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
**基础TCP**
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

**基础UDP**
```c
#include <sys/types.h>
#include <sys/socket.h>
// 由于UDP不保存状态, 每次发送数据都需要 加入目标地址.
// 不过recvfrom和sendto 也可以用于 面向STREAM的连接, 这样可以省略发送和接收端的socket地址
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
ssize_t sendto(int sockfd, const void* buf, size_t len, ing flags, const struct sockaddr* dest_addr, socklen_t addrlen);

```

**通用读写函数**

```c
#inclued <sys/socket.h>
ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags);
ssize_t sendmsg(int sockfd, struct msghdr* msg, int flags);

struct msghdr
{
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
	int msg_flags; /* 复制函数的flags参数, 并在调用过程中更新*/
};

struct iovec
{
	void* iov_base /* 内存起始地址*/
	size_t iov_len /* 这块内存长度*/
}
```
**其他Api**
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
struct linger
{
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

![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/socket%E9%80%89%E9%A1%B9.jpg)
**网络信息API**
```c
#include <netdb.h>
// 通过主机名查找ip
struct hostent* gethostbyname(const char* name);

// 通过ip获取主机完整信息 
// type为IP地址类型 AF_INET和AF_INET6
struct hostent* gethostbyaddr(const void* addr, size_t len, int type);

struct hostent
{
  char *h_name;			/* Official name of host.  */
  char **h_aliases;		/* Alias list.  */
  int h_addrtype;		/* Host address type.  */
  int h_length;			/* Length of address.  */
  char **h_addr_list;		/* List of addresses from name server.  */
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("非法输入\n");
        exit(0);
    }
    char* name = argv[1];

    struct hostent *hostptr{};

    hostptr = gethostbyname(name);
    if (hostptr == nullptr)
    {
        printf("输入存在错误 或无法获取\n");
        exit(0);
    }

    printf("Official name of hostptr: %s\n", hostptr->h_name);

    char **pptr;
    char inet_addr[INET_ADDRSTRLEN];

    printf("Alias list:\n");
    for (pptr = hostptr->h_aliases; *pptr != nullptr; ++pptr)
    {
        printf("\t%s\n", *pptr);
    }

    switch (hostptr->h_addrtype)
    {
        case AF_INET:
        {
            printf("List of addresses from name server:\n");
            for (pptr = hostptr->h_addr_list; *pptr != nullptr; ++pptr)
            {
                printf("\t%s\n",
                        inet_ntop(hostptr->h_addrtype, *pptr, inet_addr, sizeof(inet_addr)));
            }
            break;
        }
        default:
        {
            printf("unknow address type\n");
            exit(0);
        }
    }
    return 0;
}

/*
./run baidu.com
Official name of hostptr: baidu.com
Alias list:
List of addresses from name server:
	39.156.69.79
	220.181.38.148
*/
```
以下两个函数通过读取/etc/services文件 来获取服务信息 以下内容来自维基百科

Service文件是现代操作系统在etc目录下的一个配置文件，记录网络服务名对应的端口号与协议 其用途如下
- 通过TCP/IP的API函数（声明在netdb.h中）直接查到网络服务名与端口号、使用协议的对应关系。如getservbyname("serve","tcp")获取端口号;getservbyport（htons（port），“tcp”）获取端口和协议上的服务名
- 如果用户在这个文件中维护所有使用的网络服务名字、端口、协议，那么可以一目了然的获悉哪些端口号用于哪个服务，哪些端口号是空闲的
```c
#include <netdb.h>
// 根据名称获取某个服务的完整信息
struct servent getservbyname(const char* name, const char* proto);

// 根据端口号获取服务信息
struct servent getservbyport(int port, const char* proto);

struct servent
{
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

struct addrinfo
{
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

## 第六章高级IO函数

Linux提供的高级IO函数, 自然是特定条件下能力更强, 不然要他干啥, 特定条件自然限制了他的使用频率
*文件描述符*
文件描述符在是一个非负整数。是一个索引值,指向内核为每一个进程所维护的该进程打开文件的记录表。
STDOUT_FILENO(值为1)- 值为1的文件描述符为标准输出, 关闭STDOUT_FILENO后用dup即可返回最小可用值(目前为, 1) 这样输出就重定向到了调用dup的参数指向的文件

### 创建文件描述符 - pipe dup dup2 splice select
**pipe函数**
这个函数可用于创建一个管道, 实现进程间的通信. 

```c
// 函数定义
// 参数文件描述符数组 fd[0] 读出 fd[1]写入 单向管道
// 成功返回0, 并将一对打开的文件描述符填入其参数指向的数组
// 失败返回-1 errno
#include <unistd.h>
int pipe(int fd[2]);
```
```c
// 双向管道
// 第一个参数为 协议PF_UNIX(书上是AF_UNIX)感觉这里指明协议使用PF更好一些
#include <sys/types.h>
#include <sys/socket.h>
int socketpair(int domain, int type, int protocol, int fd[2]);
```
学习了后面的内容了解到了进程间通信, 回来补上一个例子
```c
int main()
{
    int fds[2];
    socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    int pid = fork();
    if (pid == 0)
    {
        close(fds[0]);
        char a[] = "123";
        send(fds[1], a, strlen(a), 0);
    }
    else if (pid > 0)
    {
        close(fds[1]);
        char b[20] {};
        recv(fds[0], b, 20, 0);
        printf("%s", b);
    }
}
```
**dup和dup2函数**
复制一个现有的文件描述符
```c
#include <unistd.h>
// 返回的文件描述符总是取系统当前可用的最小整数值
int dup(int oldfd);
// 可以用newfd来制定新的文件描述符, 如果newfd已经被打开则先关闭
// 如果newfd==oldfd 则不关闭newfd直接返回
int dup2(int oldfd, int newfd);
```
dup函数创建一个新的文件描述符, 新的文件描述符和原有的file_descriptor共同指向相同的目标.
回来补上例子, 这个例子由于关掉了`STDOUT_FILENO`dup最小的即为`STDOUT_FILENO`所以
标准输出都到了这个文件之中
```c
int main()
{
    int filefd = open("/home/lsmg/1.txt", O_WRONLY);
    close(STDOUT_FILENO);
    dup(filefd);
    printf("123\n");
    exit(0);
}
```

### 读写数据 - readv writev mmap munmap
**readv/writev**
```c
#include <sys/uio.h>
// count 为 vector的长度, 即为有多少块内存
// 成功时返回写入\读取的长度 失败返回-1
ssize_t readv(int fd, const struct iovec* vector, int count);
ssize_t writev(int fd, const struct iovec* vector, int count);

struct iovec {
	void* iov_base /* 内存起始地址*/
	size_t iov_len /* 这块内存长度*/
}
```
回来补上一个使用例子, 这个例子将一个int的内存表示写入到了文件之中
使用hexdump查看这个文件`0000000 86a0 0001`可以看到`186a0`即为100000
```c
// 2020年1月7日16:52:11
int main()
{
    int file = open("/home/lsmg/1.txt", O_WRONLY);
    int temp = 100000;
    iovec temp_iovec{};
    temp_iovec.iov_base = &temp;
    temp_iovec.iov_len = sizeof(temp);
    writev(file, &temp_iovec, 1);
}
```
sendfile函数
```c
#include <sys/sendfile.h>
// offset为指定输入流从哪里开始读, 如果为NULL 则从开头读取
ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count);

O_RDONLY只读模式
O_WRONLY只写模式
O_RDWR读写模式
int open(file_name, flag);
```
stat结构体, 可用fstat生成, **简直就是文件的身份证**
```c
#include <sys/stat.h>
struct stat
{
    dev_t       st_dev;     /* ID of device containing file -文件所在设备的ID*/
    ino_t       st_ino;     /* inode number -inode节点号*/
    mode_t      st_mode;    /* protection -保护模式?*/
    nlink_t     st_nlink;   /* number of hard links -链向此文件的连接数(硬连接)*/
    uid_t       st_uid;     /* user ID of owner -user id*/
    gid_t       st_gid;     /* group ID of owner - group id*/
    dev_t       st_rdev;    /* device ID (if special file) -设备号，针对设备文件*/
    off_t       st_size;    /* total size, in bytes -文件大小，字节为单位*/
    blksize_t   st_blksize; /* blocksize for filesystem I/O -系统块的大小*/
    blkcnt_t    st_blocks;  /* number of blocks allocated -文件所占块数*/
    time_t      st_atime;   /* time of last access -最近存取时间*/
    time_t      st_mtime;   /* time of last modification -最近修改时间*/
    time_t      st_ctime;   /* time of last status change - */
};
```
**身份证**生成函数
```c
// 第一个参数需要调用open生成文件描述符
// 下面其他两个为文件全路径
int fstat(int filedes, struct stat *buf);

// 当路径指向为符号链接的时候, lstat为符号链接的信息. stat为符号链接指向文件信息
int stat(const char *path, struct stat *buf);
int lstat(const char *path, struct stat *buf);

/*
* ln -s source dist  建立软连接, 类似快捷方式, 也叫符号链接
* ln source dist  建立硬链接, 同一个文件使用多个不同的别名, 指向同一个文件数据块, 只要硬链接不被完全
* 删除就可以正常访问
* 文件数据块 - 文件的真正数据是一个文件数据块, 打开的`文件`指向这个数据块, 就是说
* `文件`本身就类似快捷方式, 指向文件存在的区域.
*/
```
 **mmap和munmap函数**

`mmap`创建一块进程通讯共享的内存(可以将文件映射入其中), `munmap`释放这块内存
```c
#include <sys/mman.h>

// start 内存起始位置, 如果为NULL则系统分配一个地址 length为长度
// port参数 PROT_READ(可读) PROT_WRITE(可写) PROT_EXEC(可执行), PROT_NONE(不可访问)
// flag参数 内存被修改后的行为
// - MAP_SHARED 进程间共享内存, 对内存的修改反映到映射文件中
// - MAP_PRIVATE 为调用进程私有, 对该内存段的修改不会反映到文件中
// - MAP_ANONUMOUS 不是从文件映射而来, 内容被初始化为0, 最后两个参数被忽略
// 成功返回区域指针, 失败返回 -1
void* mmap(void* start, size_t length, int port, int flags, int fd, off_t offset);
// 成功返回0 失败返回-1
int munmap(void* start, size_t length);
```
**splice函数**
用于在两个文件名描述符之间移动数据, 0拷贝操作
```c
#include <fcntl.h>
// fd_in 为文件描述符, 如果为管道文件描述符则 off_in必须为NULL, 否则为读取开始偏移位置
// len为指定移动的数据长度, flags参数控制数据如何移动.
// - SPLICE_F_NONBLOCK 非阻塞splice操作, 但会受文件描述符自身的阻塞
// - SPLICE_F_MORE 给内核一个提示, 后续的splice调用将读取更多的数据???????
ssize_t splice(int fd_in, loff_t* off_in, int fd_out, loff_t* off_out, size_t len, unsigned int flags);

// 使用splice函数  实现echo服务器
int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        printf("the parmerters is wrong\n");
        exit(errno);
    }
    char *ip = argv[1];

    int port = atoi(argv[2]);
    printf("the port is %d the ip is %s\n", port, ip);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int ret = bind(sockfd, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sockfd, 5);

    int clientfd{};
    sockaddr_in client_address{};
    socklen_t client_addrlen = sizeof(client_address);

    clientfd = accept(sockfd, (sockaddr*)&client_address, &client_addrlen);
    if (clientfd < 0)
    {
        printf("accept error\n");
    }
    else
    {
        printf("a new connection from %s:%d success\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        int fds[2];
        pipe(fds);
        ret = splice(clientfd, nullptr, fds[1], nullptr, 32768, SPLICE_F_MORE);
        assert(ret != -1);

        ret = splice(fds[0], nullptr, clientfd, nullptr, 32768, SPLICE_F_MORE);
        assert(ret != -1);

        close(clientfd);
    }
    close(sockfd);
    exit(0);
}
```

**select 函数**
select函数在第二个参数列表 可读的时候返回
或者是等到了规定的时间返回

返回之后 第二个参数指向fdset的集合 被修改为可读的fd列表
这就需要每次返回后都更新 fdset集合

返回后 此函数的返回值为可读的fd数量, 遍历fdset集合 同时使用FD_ISSET判断fdset[i] 是否在其中
然后判断此fd是否为listenfd 如果是则接受新的连接 如果不是说明是已经接受的其他fd 判断是有数据可读
还是此连接断开

```c
#include <fcntl.h> 
// maxfdp 最大数 FD_SETSIZE
// struct fd_set 一个集合,可以存储多个文件描述符
// - FD_ZERO(&fd_set) 清空 -FD_SET(fd, &fd_set) 放入fd FD_CLR(fd, &fd_set)从其中清除fd
// - FD_ISSET(fd, &fd_set) 判断是否在其中
// readfds  需要监视的文件描述符读变化, 其中的文件描述符可读的时候返回
// writefds 需要监视的文件描述符写变化, 其中的文件描述符可写的时候返回
// errorfds 错误
// timeout 传入NULL为阻塞, 设置为0秒0微秒则变为非阻塞函数
// 返回值 负值为错误 等待超时说明文件无变化返回0 有变化返回正值
int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval*timeout); 

#define exit_if(r, ...) \
{   \
    if (r)  \
    {   \
        printf(__VA_ARGS__);    \
        printf("errno no: %d, error msg is %s", errno, strerror(errno));    \
        exit(1);    \
    }   \
}   \

int main(int argc, char* argv[])
{
    int keyboard_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
    exit_if(keyboard_fd < 0, "open keyboard fd error\n");
    fd_set readfd;
    char recv_buffer = 0;

    while (true)
    {
        FD_ZERO(&readfd);
        FD_SET(0, &readfd);

        timeval timeout {5, 0};

        int ret = select(keyboard_fd + 1, &readfd, nullptr, nullptr, &timeout);
        exit_if(ret == -1, "select error\n");
        if (ret > 0)
        {
            if (FD_ISSET(keyboard_fd, &readfd))
            {
                recv_buffer = 0;
                read(keyboard_fd, &recv_buffer, 1);
                if ('\n' == recv_buffer)
                {
                    continue;
                }
                if ('q' == recv_buffer)
                {
                    break;
                }
                printf("the input is %c\n", recv_buffer);
            }

        }
        if (ret == 0)
        {
            printf("timeout\n");
        }
    }
}
```
## 第七章Linux服务器程序规范

- Linux程序服务器 一般以后台进程形式运行.  后台进程又称为守护进程(daemon). 他没有控制终端, 因而不会意外的接收到用户输入. 守护进程的父进程通常都是init进程(PID为1的进程)
- Linux服务器程序有一套日志系统, 他至少能输出日志到文件. 日志这东西太重要了,排错对比全靠它.
- Linux服务器程序一般以某个专门的非root身份运行. 比如mysqld有自己的账户mysql.
- Linux服务器程序一般都有自己的配置文件, 而不是把所有配置都写死在代码里面, 方便后续的更改.
- Linux服务器程序通常在启动的时候生成一个PID文件并存入/var/run 目录中, 以记录改后台进程的PID.
- Linux服务器程序通常需要考虑系统资源和限制, 预测自己的承受能力

### 日志

```shell
sudo service rsyslog restart // 启动守护进程
```
```c
#include <syslog.h>
// priority参数是所谓的设施值(记录日志信息来源, 默认为LOG_USER)与日志级别的按位或
// - 0 LOG_EMERG  /* 系统不可用*/
// - 1 LOG_ALERT   /* 报警需要立即采取行动*/
// - 2 LOG_CRIT /* 非常严重的情况*/
// - 3 LOG_ERR  /* 错误*/
// - 4 LOG_WARNING /* 警告*/
// - 5 LOG_NOTICE /* 通知*/
// - 6 LOG_INFO /* 信息*/
//  -7 LOG_DEBUG /* 调试*/
void syslog(int priority, const char* message, .....);

// ident 位于日志的时间后 通常为名字
// logopt 对后续 syslog调用的行为进行配置
// -  0x01 LOG_PID  /* 在日志信息中包含程序PID*/
// -  0x02 LOG_CONS /* 如果信息不能记录到日志文件, 则打印到终端*/
// -  0x04 LOG_ODELAY /* 延迟打开日志功能直到第一次调用syslog*/
// -  0x08 LOG_NDELAY /* 不延迟打开日志功能*/
// facility参数可以修改syslog函数中的默认设施值
void openlog(const char* ident, int logopt, int facility);

// maskpri 一共八位 0000-0000
// 如果将最后一个0置为1 表示 记录0级别的日志
// 如果将最后两个0都置为1 表示记录0和1级别的日志
// 可以通过LOG_MASK() 宏设定 比如LOG_MASK(LOG_CRIT) 表示将倒数第三个0置为1, 表示只记录LOG_CRIT
// 如果直接设置setlogmask(3); 3的二进制最后两个数均为1 则记录 0和1级别的日志
int setlogmask(int maskpri);

// 关闭日志功能
void closelog();
```

### 用户信息, 切换用户
UID - 真实用户ID
EUID - 有效用户ID - 方便资源访问
GID - 真实组ID
EGID - 有效组ID
```c
#include <sys/types.h>
#include <unistd.h>

uid_t getuid();
uid_t geteuid();
gid_t getgid();
gid_t getegid();
int setuid(uid_t uid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setegid(gid_t gid);
```

可以通过 `setuid`和`setgid`切换用户 **root用户uid和gid均为0**

### 进程间关系
PGID - 进程组ID(Linux下每个进程隶属于一个进程组)

#include <unistd.h>
pid_t getpgid(pid_t pid); 成功时返回pid所属的pgid 失败返回-1
int setpgid(pid_t pid, pid_t pgid);

**会话**
一些有关联的进程组将形成一个会话
略过

**查看进程关系**
ps和less

**资源限制**
略
**改变目录**
略

## 第八章高性能服务器程序框架

**服务器模型-CS模型**

**优点**
- 实现起来简单
**缺点**
- 服务器是通信的中心, 访问过大的时候会导致响应过慢

模式图
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%9B%BE8-2%20TCP%E6%9C%8D%E5%8A%A1%E5%99%A8%E5%92%8C%E5%AE%A2%E6%88%B7%E7%AB%AF%E5%B7%A5%E4%BD%9C%E6%B5%81%E7%A8%8B.png)

编写的demo 没有用到fork函数. 后续待完善

**服务器框架 IO模型**

![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E6%9C%8D%E5%8A%A1%E5%99%A8%E5%9F%BA%E6%9C%AC%E6%A1%86%E6%9E%B6.png)

这个模型大概能够理解, 自己也算是学了半年的Javaweb.

socket在创建的时候默认是阻塞的, 不过可以通过传`SOCK_NONBLOCK`参解决
非阻塞调用都会立即返回 但可能事件没有发生(recv没有接收到信息), 没有发生和出错都会`返回-1` 所以需要通过`errno`来区分这些错误.
**事件未发生**
accept, send,recv errno被设置为 `EAGAIN(再来一次)`或`EWOULDBLOCK(期望阻塞)`
connect 被设置为 `EINPROGRESS(正在处理中)`

需要在事件已经发生的情况下 去调用非阻塞IO, 才能提高性能

常用IO复用函数 `select` `poll` `epoll_wait` 将在第九章后面说明
信号将在第十章说明

**两种高效的事件处理模式和并发模式**
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/Reactor%E6%A8%A1%E5%BC%8F.png)

![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/Proactor%E6%A8%A1%E5%BC%8F.png)

![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E7%94%A8%E5%90%8C%E6%AD%A5IO%E6%A8%A1%E6%8B%9F%E5%87%BA%E7%9A%84Proactor%E6%A8%A1%E5%BC%8F.png)

程序分为计算密集型(CPU使用很多, IO资源使用很少)和IO密集型(反过来).
前者使用并发编程反而会降低效率, 后者则会提升效率
并发编程有多进程和多线程两种方式

并发模式 - IO单元和多个逻辑单元之间协调完成任务的方法.
服务器主要有两种并发模式
- 半同步/半异步模式
- 领导者/追随者模式

**半同步/半异步模式**
在IO模型中, 异步和同步的区分是内核向应用程序通知的是何种IO事件(就绪事件还是完成事件), 以及由谁来完成IO读写(应用程序还是内核)

而在这里(并发模式) 
同步指的是完全按照代码序列的顺序执行 - 按照同步方式运行的线程称为同步线程
异步需要系统事件(中断, 信号)来驱动 - 按照异步方式运行的线程称为异步线程
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%B9%B6%E5%8F%91%E6%A8%A1%E5%BC%8F%E4%B8%AD%E7%9A%84%E5%BC%82%E6%AD%A5%E5%92%8C%E5%90%8C%E6%AD%A5.png)

服务器(需要较好的实时性且能同时处理多个客户请求) - 一般使用同步线程和异步线程来实现,即为半同步/半异步模式
同步线程 - 处理客户逻辑, 处理请求队列中的对象
异步线程 - 处理IO事件, 接收到客户请求后将其封装成请求对象并插入请求队列

半同步/半异步模式 存在变体 `半同步/半反应堆模式`
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%8D%8A%E5%90%8C%E6%AD%A5%E5%8D%8A%E5%8F%8D%E5%BA%94%E5%A0%86%E6%A8%A1%E5%BC%8F.png)

异步线程 - 主线程 - 负责监听所有socket上的事件

**领导者/追随者模式**
略

**高效编程方法 - 有限状态机**
```c
// 状态独立的有限状态机
STATE_MACHINE(Package _pack) {
	
	PackageType _type = _pack.GetType();
	switch(_type) {
		case type_A:
			xxxx;
			break;
		case type_B:
			xxxx;
			break;
	}
}

// 带状态转移的有限状态机
STATE_MACHINE() {
	State cur_State = type_A;
	while(cur_State != type_C) {
	
		Package _pack = getNewPackage();
		switch(cur_State) {
			
			case type_A:
				process_package_state_A(_pack);
				cur_State = type_B;
				break;
			case type_B:
				xxxx;
				cur_State = type_C;
				break;
		}
	}
}
```

花了小一个小时 终于一个字母一个字母的抄完了那个5000多字的代码
@2019年9月8日22:08:46@

### 提高服务器性能的其他建议 池 数据复制 上下文切换和锁

**池** - 用空间换取时间
进程池和线程池

**数据复制** - 高性能的服务器应该尽量避免不必要的复制

**上下文切换和锁**
减少`锁`的作用区域. 不应该创建太多的工作进程, 而是使用专门的业务逻辑线程.

## 第九章 I/O复用

I/O复用使得程序能同时监听多个文件描述符.
- 客户端程序需要同时处理多个socket 非阻塞connect技术
- 客户端程序同时处理用户输入和网络连接 聊天室程序
- TCP服务器要同时处理监听socket和连接socket
- 同时处理TCP和UDP请求 - 回射服务器
- 同时监听多个端口, 或者处理多种服务 - xinetd服务器

常用手段`select`, `poll`, `epoll`

### select
```c++
#inlcude <sys/select.h>
// nfds - 被监听的文件描述符总数
// 后面三个分别指向 可读, 可写, 异常等事件对应的文件描述符集合
// timeval select超时时间 如果传递0 则为非阻塞, 设置为NULL则为阻塞
// 成功返回就绪(可读, 可写, 异常)文件描述符的总数, 没有则返回0 失败返回-1
int select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);

//操作fd_set的宏
FD_ZERO(fd_set* fdset);
FD_SET(int fd, fd_set* fdset);
FD_CLR(int fd, fd_set* fdset);
FD_ISSET(int fd, fd_set* fdset);
// 设置 timeval 超时时间
struct timeval
{
	long tv_sec; // 秒
	long tv_usec; // 微秒
}
```
**select**

文件描述符就绪条件
- socket内核接收缓存区中的字节数大于或等于 其低水位标记
- socket通信的对方关闭连接, 对socket的读操作返回0
- 监听socket上有新的连接请求
- socket上有未处理的错误, 可以使用getsockopt来读取和清除错误
- socket内核的发送缓冲区的可用字节数大于或等于 其低水位标记
- socket的写操作被关闭, 对被关闭的socket执行写操作将会触发一个SIGPIPE信号
- socket使用非阻塞connect 连接成功或失败后
### poll
**poll**
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/poll%E6%97%B6%E9%97%B4%E7%B1%BB%E5%9E%8B1.png)
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/poll%E6%97%B6%E9%97%B4%E7%B1%BB%E5%9E%8B2.png)

```c++
#include <poll.h>
// fds 结构体类型数组 指定我们感兴趣的文件描述符上发生的可读可写和异常事件\
// nfds 遍历结合大小 左闭右开
// timeout 单位为毫秒 -1 为阻塞 0 为立即返回
int poll(struct pollfd* fds, nfds_t nfds, int timeout);

struct pollfd
{
	int fd;
	short events;  //注册的事件, 告知poll监听fd上的哪些事件
	short revents; // 实际发生的事件
}
```
```c++
#define exit_if(r, ...) \
{   \
    if (r)  \
    {   \
        printf(__VA_ARGS__);    \
        printf("errno no: %d, error msg is %s", errno, strerror(errno));    \
        exit(1);    \
    }   \
}   \

struct client_info
{
    char *ip_;
    int port_;
};

int main(int argc, char* argv[])
{
    int port = 8001;
    char ip[] = "127.0.0.1";

    struct sockaddr_in address;
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htons(INADDR_ANY);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    exit_if(listenfd < 0, "socket error\n");

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    exit_if(ret == -1, "bind error\n");

    ret = listen(listenfd, 5);
    exit_if(ret == -1, "listen error\n");

    constexpr int MAX_CLIENTS = 1024;
    struct pollfd polls[MAX_CLIENTS] = {};
    struct client_info clientsinfo[MAX_CLIENTS] = {};

    polls[3].fd = listenfd;
    polls[3].events = POLLIN | POLLRDHUP;


    while (true)
    {
        ret = poll(polls, MAX_CLIENTS + 1, -1);
        exit_if(ret == -1, "poll error\n");

        for (int i = 3; i <= MAX_CLIENTS; ++i)
        {
            int fd = polls[i].fd;

            if (polls[i].revents & POLLRDHUP)
            {
                polls[i].events = 0;
                printf("close fd-%d from %s:%d\n", fd, clientsinfo[fd].ip_, clientsinfo[fd].port_);
            }

            if (polls[i].revents & POLLIN)
            {
                if (fd == listenfd)
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addresslen = sizeof(client_address);

                    int clientfd = accept(listenfd, (struct sockaddr*)&client_address,
                            &client_addresslen);

                    struct client_info *clientinfo = &clientsinfo[clientfd];

                    clientinfo->ip_ = inet_ntoa(client_address.sin_addr);
                    clientinfo->port_ = ntohs(client_address.sin_port);

                    exit_if(clientfd < 0, "accpet error, from %s:%d\n", clientinfo->ip_,
                            clientinfo->port_);
                    printf("accept from %s:%d\n", clientinfo->ip_, clientinfo->port_);

                    polls[clientfd].fd = clientfd;
                    polls[clientfd].events = POLLIN | POLLRDHUP;
                }
                else
                {
                    char buffer[1024];
                    memset(buffer, '\0', sizeof(buffer));

                    ret = read(fd, buffer, 1024);
                    if(ret == 0)
                    {
                        close(fd);
                    }
                    else
                    {
                        printf("recv from %s:%d:\n%s\n", clientsinfo[fd].ip_,
                               clientsinfo[fd].port_, buffer);
                    }
                }
            }
        }
    }
}
```
### epoll
**epoll**

epoll是Linux特有的I/O复用函数, 实现上与select,poll有很大的差异
- epoll使用一组函数完成任务
- epoll把用户关心的文件描述符上的事件放在内核里的一个事件表中
- epoll无需每次调用都传入文件描述符集或事件集.

有特定的文件描述符创建函数, 来标识这个事件表`epoll_create()`
`epoll_ctl()` 用来操作这个内核事件表
`epoll_wait()` 为主要函数 成功返回就绪的文件描述符个数 失败返回-1
如果`epoll_wait()`函数检测到事件,就将所有就绪的事件从内核事件表(由第一个参数, epoll_create返回的结果) 中复制到第二个参数event指向的数组中, 这个数组只用于输出`epoll_wait`检测到的就绪事件.

*event不同于select和poll的数组参数 既用于传入用户注册的事件, 又用于输出内核检测到的就绪事件, 提高了效率*

```c++
// 索引poll返回的就绪文件描述符
int ret = poll(fds, MAX_EVENT_NUMBER - 1);
// 遍历
for(int i = 0; i < MAX_EVENT_NUMBER; ++i) {
	if(fds[i].revents & POLLIN) {
		int sockfd = fds[i].fd;
	}
}

// 索引epoll返回的就绪文件描述符
int ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER,  -1);
for(int i = 0; i < ret; i++) {
	int sockfd = events[i].data.fd;
	// sockfd 一定就绪 ?????
}
```

**LT和ET模式**
LT(电平触发, 默认的工作模式)
LT模式下的epoll相当于一个效率较高的poll
epoll_wait将会一只通知一个事件知道这个事件被处理

ET(边沿触发, epoll的高效工作模式)模式
当向epoll内核事件表中注册一个文件描述符上的EPOLLET事件的时候, epoll将用ET模式来操作这个
文件描述符
epoll_wait只会通知一次, 不论这个事件有没有完成

ET模式
```
-> 123456789-123456789-123456789
event trigger once
get 9bytes of content: 123456789
get 9bytes of content: -12345678
get 9bytes of content: 9-1234567
get 4bytes of content: 89
read later
```
LT模式
```
-> 123456789-123456789-123456789
event trigger once
get 9bytes of contents: 123456789
event trigger once
get 9bytes of contents: -12345678
event trigger once
get 9bytes of contents: 9-1234567
event trigger once
get 4bytes of contents: 89
```
ET模式有任务到来就必须做完, 因为后续将不会继续通知这个事件, 所以ET是epoll的高效工作模式
LT模式只要事件没被处理就会一直通知

```c++
#include <epoll.h>
// size 参数只是给内核一个提示, 事件表需要多大
// 函数返回其他所有epoll系统调用的第一个参数, 来指定要访问的内核事件表
int epoll_create(int size);

// epfd 为 epoll_create的返回值
// op为操作类型
// - EPOLL_CTL_ADD 向事件表中注册fd上的事件
// - EPOLL_CTL_MOD 修改fd上的注册事件
// - EPOLL_CTL_DEL 删除fd上的注册事件
// fd 为要操作的文件描述符
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);

struct epoll_event
{
	_uint32_t events; // epoll事件
	epoll_data_t data; // 用户数据 是一个联合体
}

typedef union epoll_data
{
	void* ptr; // ptr fd 不能同时使用
	int fd;
	uint32_t u32;
	uint64_t u64;
}epoll_data_t

// maxevents监听事件数 必须大于0
// timeout 为-1 表示阻塞
// 成功返回就绪的文件描述符个数 失败返回-1
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
```

### 三种IO复用的比较
`select`以及`poll`和`epoll`
相同
- 都能同时监听多个文件描述符, 都将等待timeout参数指定的超时时间, 直到一个或多个文件描述符上有事件发生.
- 返回值为就绪的文件描述符数量, 返回0则表示没有事件发生
- ![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E4%B8%89%E7%A7%8DIO%E5%A4%8D%E7%94%A8%E6%AF%94%E8%BE%83.png)

### I/O 复用的高级应用, 非阻塞connect

connect出错的时候会返回一个errno值 EINPROGRESS - 表示对非阻塞socket调用connect, 连接又没有立即建立的时候, 这时可以调用select和poll函数来监听这个连接失败的socket上的可写事件.

当函数返回的时候, 可以用getsockopt来读取错误码, 并清楚该socket上的错误. 错误码为0表示成功


## 第十章信号

### Api
发送信号Api
```c++
#include <sys/types.h>
#include <signal.h>

// pid > 0 发送给PID为pid标识的进程
//  0 发送给本进程组的其他进程
// -1 发送给进程以外的所有进程, 但发送者需要有对目标进程发送信号的权限
// < -1 发送给组ID为 -pid 的进程组中的所有成员

// 出错信息 EINVAL 无效信号, EPERM 该进程没有权限给任何一个目标进程 ESRCH 目标进程(组) 不存在
int kill(pid_t pid, int sig);
```
接收信号Api
```c++
#include <signal.h>
typedef void(*_sighandler_t) (int);

#include <bits/signum.h> // 此头文件中有所有的linux可用信号
// 忽略目标信号
#define SIG_DFL ((_sighandler_t) 0)
// 使用信号的默认处理方式
#define SIG_IGN ((_sighandler_t) 1)
```
常用信号
```
SIGHUP 控制终端挂起
SIGPIPE 往读端被关闭的管道或者socket连接中写数据
SIGURG socket连接上收到紧急数据
SIGALRM 由alarm或setitimer设置的实时闹钟超时引起
SIGCHLD 子进程状态变化
```
信号函数
```c++
// 为一个信号设置处理函数
#include <signal.h>
// _handler 指定sig的处理函数
_sighandler_t signal(int sig, __sighandler_t _handler)


int sigaction(int sig, struct sigaction* act, struct sigaction* oact)
```
### 概述

信号是用户, 系统, 或者进程发送给目标进程的信息, 以通知目标进程某个状态的改变或者系统异常.
产生条件
- 对于前台进程
用户可以通过输入特殊的终端字符来给它发送信号, CTRL+C 通常为一个中断信号 `SIGINT`
- 系统异常
浮点异常和非法内存段的访问
- 系统状态变化
由alarm定时器到期将引起`SIGALRM`信号
- 运行kill命令或调用kill函数

*服务器必须处理(至少忽略) 一些常见的信号, 以免异常终止*

中断系统调用?

## 第十一章定时器
### socket选项`SO_RCVTIMEO` 和 `SO_SNDTIMEO`
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/SO_RCVTIMEO%E5%92%8CSO_SNDTIMEO%E9%80%89%E9%A1%B9%E7%9A%84%E4%BD%9C%E7%94%A8.png)

使用示例, 通过设置对应的SO_SNDTIMEO 得到超时后的路线
```c++
int timeout_connect(const char* ip, const int port, const int sec)
{
    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    exit_if(sockfd < 0, "socket error\n");

    struct timeval timeout{};
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    socklen_t timeout_len = sizeof(timeout);

    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeout_len);

    int ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    if (ret == -1)
    {
		// 当 errno为EINPROGRESS 说明 等待了 10S后依然无法连接成功 实现了定时器
        if (errno == EINPROGRESS)
        {
            printf("connecting timeout, process timeout logic\n");
            return -1;
        }
        printf("error occur when connecting to server\n");
        return -1;
    }
    return sockfd;
}

int main(int argc, char* argv[])
{
    exit_if(argc <= 2, "wrong number of parameters\n")
    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int sockfd = timeout_connect(ip, port, 10);
    if (sockfd < 0)
    {
        return 1;
    }
    return 0;
}
```

### SIGALRM信号-基于升序链表的定时器
由alarm和setitimer函数设定的实时闹钟一旦超时, 将会触发SIGALRM信号, 用信号处理函数处理定时任务


### IO复用系统调用的超时参数

### 高性能定时器

#### 时间轮

#### 时间堆