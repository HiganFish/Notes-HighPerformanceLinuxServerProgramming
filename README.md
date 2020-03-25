# 第一章 TCP/IP协议族

## TCP/IP协议族体系结构和主要协议
协议族中协议众多, 这本书只选取了IP和TCP协议 - 对网络编程影响最直接

![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%9B%9B%E5%B1%82%E7%BB%93%E6%9E%84.jpg)

![](https://gss1.bdstatic.com/9vo3dSag_xI4khGkpoWK1HF6hhy/baike/c0%3Dbaike80%2C5%2C5%2C80%2C26/sign=3a1768f4c6fc1e17e9b284632bf99d66/0dd7912397dda144d48ab350bbb7d0a20df48655.jpg)

同样七层是osi参考模型, 简化后得到四层
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
TCP协议(Transmission Control Protocol 传输控制协议)
- 为应用层提供`可靠的, 面向连接, 基于流的服务`
- 通过`超时重传`和`数据确认`等确保数据正常送达.
- TCP需要存储一些必要的状态, 连接的状态, 读写缓冲区, 诸多定时器
UPD协议(User Datagram Protocol 用户数据报协议)
- 为应用层提供`不可靠的, 无连接的, 基于数据报的服务`
- 一般需要自己处理`数据确认`和`超时重传`的问题
- 通信两者不存储状态, 每次发送都需要指定地址信息. `有自己的长度`

**网络层**
实现了数据包的选路和转发.  只有数据包到不了目标地址, 就`下一跳`(hop by hop), 选择最近的.
*IP协议(Internet Protocol)* 以及 *ICMP协议(Internet Control Message Protocol)* 
后者协议是IP协议的补充, 用来检测网络连接 1. 差错报文, 用来回应状态 2. 查询报文(ping程序就是使用的此报文来判断信息是否送达)

**数据链路层**
实现了网卡接口的网络驱动程序. 这里驱动程序方便了厂商的下层修改, 只需要向上层提供规定的接口即可.
存在两个协议 *ARP协议(Address Resolve Protocol, 地址解析协议)*. 还有*RARP(Reverse ~, 逆地址解析协议)*.  由于网络层使用IP地址寻址机器, 但是数据链路层使用物理地址(通常为MAC地址), 之间的转化涉及到ARP协议*ARP欺骗, 可能与这个有关, 目前不去学习*

**封装**
上层协议发送到下层协议. 通过封装实现, 层与层之间传输的时候, 加上自己的头部信息.
被TCP封装的数据成为 `TCP报文段`
- 内核部分发送成功后删除数据

被UDP封装的数据成为 `UDP数据报`
- 发送后即删除

再经IP封装后成为`IP数据报`
最后经过数据链路层封装后为 `帧`

以太网最大数据帧1518字节 抛去14头部 帧尾4校验
MTU: 帧的最大传输单元 一般为1500字节
MSS: TCP数据包最大的数据载量 1460字节 = 1500字节 - 20Ip头-20TCP头 还有额外的40字节可选部分

**ARP**
ARP协议能实现任意网络层地址到任意物理地址的转换

# 第二章 IP协议详解
IP协议是TCP/IP协议簇的核心协议, 是socket网络编程的基础之一
IP协议为上层协议提供无状态, 无连接, 不可靠的服务

IP数据报最大长度是65535(2^16 - 1)字节, 但是有MTU的限制

当IP数据报的长度超过MTU 将会被分片传输. 分片可能发生在发送端, 也可能发生在中转路由器, 还可能被多次分片. 只有在最终的目标机器上, 这些分片才会被内核中的ip模块重新组装

![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E6%90%BA%E5%B8%A6ICMP%E6%8A%A5%E6%96%87%E7%9A%84IP%E6%95%B0%E6%8D%AE%E6%8A%A5%E8%A2%AB%E5%88%86%E7%89%87.png)


路由机制


![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E8%B7%AF%E7%94%B1%E6%9C%BA%E5%88%B6.png)


给定了目标IP地址后, 将会匹配路由表中的哪一项呢? 分三个步骤
- 查找路由表中和数据报的目标IP地址完全匹配的主机IP地址. 如果找到就使用该路由项. 否则下一步
- 查找路由表中和数据报的目标IP地址具有相同网路ID的网络IP地址 找到....... 否则下一步
- 选择默认路由项, 通常意味着下一跳路由是网关


# 第三章 TCP协议详解

Tcp读写都是针对缓冲区来说, 所以没有固定的读写次数对应关系.

UDP没有缓冲区, 必须及时接受数据否则会丢包, 或者接收缓冲区过小就会造成数据报截断

ISN-初始序号值
32位序号 后续的TCP报文段中序号值 seq = ISN + 报文段首字节在整个字节流中的偏移
32位确认号 收到的TCP报文序号值+1. 这个32位确认号每次发送的是上一次的应答

ACK标志: 表示确认号是否有效. 携带ACK标志的报文段称为`确认报文段`
PSH标志: 提示接收端应用程序从TCP接受缓冲区中读走数据, 为后续数据腾出空间
RST标志: 要求对方重新建立连接 携带......`复位报文段`
SYN标志: 标志请求建立一个连接 携带......`同步报文段`
FIN标志: 通知对方本端连接要关闭了, 携带..`结束报文段`

16位窗口大小: 窗口指的是接收通告窗口, 告诉对方本端的TCP 接收缓冲区还能容纳多少字节的数据
16位校验和: `可靠传输的重要保障`发送端填充, 接收端执行CRC算法校验是否损坏, 同时校验`TCP头部`和`数据部分`


**TCP连接的建立和关闭**

```s
# 三次握手
# 客户端发送请求连接 ISN=seq + 0 = 3683340920
# mss 最大数据载量1460
IP 192.168.80.1.7467 > ubuntu.8000: 
Flags [S], seq 3683340920, win 64240, 
options [mss 1460,nop,wscale 8,nop,nop,sackOK], length 0

# 同意客户端连接
# ack = 客户端发送 seq + 1
# 同时发送服务端的seq
IP ubuntu.8000 > 192.168.80.1.7467: 
Flags [S.], seq 938535101, ack 3683340921, win 64240, 
options [mss 1460,nop,nop,sackOK,nop,wscale 7], length 0

# 虽然这个报文段没有字节 但由于是同步报文段 需要占用一个序号值
# 这里是tcpdump的处理 ack显示相对值 即 3683340921 - 3683340920 = 1
IP 192.168.80.1.7467 > ubuntu.8000: 
Flags [.], ack 938535102, win 4106, length 0


# 包含FIN标志 说明要求结束连接 也需要占用一个序号值
IP 192.168.80.1.7467 > ubuntu.8000: 
Flags [F.], seq 1, ack 1, win 4106, length 0

# 服务端确认关闭连接
IP ubuntu.8000 > 192.168.80.1.7467: 
Flags [.], ack 2, win 502, length 0

# 服务端发送关闭连接
IP ubuntu.8000 > 192.168.80.1.7467: 
Flags [F.], seq 1, ack 2, win 4105, length 0

# 客户端确认
IP 192.168.80.1.7467 > ubuntu.8000: 
Flags [.], ack 2, win 503, length 0
```

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

# 第九章 I/O复用

I/O复用使得程序能同时监听多个文件描述符.
- 客户端程序需要同时处理多个socket 非阻塞connect技术
- 客户端程序同时处理用户输入和网络连接 聊天室程序
- TCP服务器要同时处理监听socket和连接socket
- 同时处理TCP和UDP请求 - 回射服务器
- 同时监听多个端口, 或者处理多种服务 - xinetd服务器

常用手段`select`, `poll`, `epoll`

## select
```c++
#include <sys/select.h>
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
## poll
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
## epoll
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

## 三种IO复用的比较
`select`以及`poll`和`epoll`
相同
- 都能同时监听多个文件描述符, 都将等待timeout参数指定的超时时间, 直到一个或多个文件描述符上有事件发生.
- 返回值为就绪的文件描述符数量, 返回0则表示没有事件发生
- ![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E4%B8%89%E7%A7%8DIO%E5%A4%8D%E7%94%A8%E6%AF%94%E8%BE%83.png)

## I/O 复用的高级应用, 非阻塞connect

connect出错的时候会返回一个errno值 EINPROGRESS - 表示对非阻塞socket调用connect, 连接又没有立即建立的时候, 这时可以调用select和poll函数来监听这个连接失败的socket上的可写事件.

当函数返回的时候, 可以用getsockopt来读取错误码, 并清楚该socket上的错误. 错误码为0表示成功


# 第十章信号

## Api
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
## 概述

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

# 第十一章定时器
## socket选项`SO_RCVTIMEO` 和 `SO_SNDTIMEO`
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

## SIGALRM信号-基于升序链表的定时器
由alarm和setitimer函数设定的实时闹钟一旦超时, 将会触发SIGALRM信号, 用信号处理函数处理定时任务
 相关的代码放在了github上 代码还是很多的就不放上来了[连接](https://github.com/rjd67441/Notes-HighPerformanceLinuxServerProgramming/tree/master/12.%20%E4%BB%A3%E7%A0%81%E6%B8%85%E5%8D%9511-2%E5%92%8C11-3%E5%8F%8A11-4%20%E9%93%BE%E8%A1%A8%E5%AE%9A%E6%97%B6%E5%99%A8%2C%20%E5%A4%84%E7%90%86%E9%9D%9E%E6%B4%BB%E5%8A%A8%E8%BF%9E%E6%8E%A5)

总结放在了 日记的博客上 链接后面再甩出来
## IO复用系统调用的超时参数

## 高性能定时器
## # 时间轮
## # 时间堆

# 第十二章高性能IO框架库
另出一篇博客

# 第十三章多进程编程

## exec系列系统调用
```c++
#include <unistd.h>
// 声明这个是外部函数或外部变量
extern char** environ;

// path 参数指定可执行文件的完成路径 file接收文件名,具体位置在PATH中搜寻
// arg-接受可变参数 和 argv用于向新的程序传递参数数组
// envp用于设置新程序的环境变量, 未设置则使用全局的环境变量
// exec函数是不返回的, 除非出错
// 如果未报错则源程序被新的程序完全替换

int execl(const char* path, const char* arg, ...);
int execlp(const char* file, const char* arg, ...);
int execle(const char* path, const char* arg, ..., char* const envp[])
int execv(const char* path, char* const argv[]);
int execvp(const char* file, char* const argv[]);
int execve(const char* path, char* const argv[], char* const envp[]);
```
## fork系统调用-进程的创建
```c++
#include <sys/types.h>
#include <unistd.h>
// 每次调用都返回两次, 在父进程中返回的子进程的PID, 在子进程中返回0
// 次返回值用于区分是父进程还是子进程
// 失败返回-1
pid_t fork(viod);
```
fork系统调用
fork() 函数复制当前的进程, 在内核进程表中创建一个新的进程表项
新的进程表项有很多的属性和原进程相同
- 堆指针
- 栈指针
- 标志寄存器的值
- 子进程代码与父进程完全相同
- 同时复制(采用了写时复制, 父进程和子进程对数据执行了写操作才会复制)父进程的数据(堆数据, 栈数据, 静态数据)
- 创建子进程后, *父进程打开的文件描述符默认在子进程中也是打开的* `文件描述符的引用计数`, `父进程的用户根目录, 当前工作目录等变量的引用计数` 均加1

也存在不同的项目
- 该进程的PPID(标识父进程)被设置成原进程的PID,  
- `信号位图被清除`(原进程设置的信号处理函数对新进程无效)


(引自维基百科-引用计数是计算机编程语言中的一种内存管理技术，是指将资源（可以是对象、内存或磁盘空间等等）的被引用次数保存起来，当被引用次数变为零时就将其释放的过程。)


The child process is an exact duplicate of the parent process except
for the following points:
*  The child has its own unique process ID, and this PID does not
	match the ID of any existing process group (setpgid(2)) or
	session. 子进程拥有自己唯一的进程ID, 不与其他相同

*  The child's parent process ID is the same as the parent's process
	ID. 子进程的父进程ID PPID 与父进程ID PID相同

*  The child does not inherit its parent's memory locks (mlock(2),
	mlockall(2)). 子进程不继承父进程的内存锁(保证一部分内存处于内存中, 而不是sawp分区)

*  Process resource utilizations (getrusage(2)) and CPU time counters
	(times(2)) are reset to zero in the child.
	进程资源使用和CPU时间计数器在子进程中重置为0

*  The child's set of pending signals is initially empty
	(sigpending(2)). 信号位图被初始化为空 原信号处理函数对子进程无效 需重新设置

*  The child does not inherit semaphore adjustments from its parent
	(semop(2)). 不会继承semadj

*  The child does not inherit process-associated record locks from
	its parent (fcntl(2)).  (On the other hand, it does inherit
	fcntl(2) open file description locks and flock(2) locks from its
	parent.)

*  The child does not inherit timers from its parent (setitimer(2),
	alarm(2), timer_create(2)). 不会继承定时器

*  The child does not inherit outstanding asynchronous I/O operations
	from its parent (aio_read(3), aio_write(3)), nor does it inherit
	any asynchronous I/O contexts from its parent (see io_setup(2)).




## 处理僵尸进程-进程的管理
```c++
#include <sys/types.h>
#include <sys/wait.h>
// wait进程将阻塞进程, 直到该进程的某个子进程结束运行为止. 他返回结束的子进程的PID, 并将该子进程的退出状态存储于stat_loc参数指向的内存中. sys/wait.h 头文件中定义了宏来帮助解释退出信息.
pid_t wait(int* stat_loc);

// 非阻塞, 只等待由pid指定的目标子进程(-1为阻塞)
// options函数取值WNOHANG-waitpid立即返回
// 如果目标子进程正常退出, 则返回子进程的pid
// 如果还没有结束或意外终止, 则立即返回0
// 调用失败返回-1
pid_t waitpid(pid_t pid, int* stat_loc, int options);

WIFEXITED(stat_val); // 子进程正常结束, 返回一个非0
WEXITSTATUS(stat_val); // 如果WIFEXITED 非0, 它返回子进程的退出码
WIFSIGNALED(stat_val);// 如果子进程是因为一个未捕获的信号而终止, 返回一个非0值
WTERMSIG(stat_val);// 如果WIFSIGNALED非0 返回一个信号值
WIFSTOPPED(stat_val);// 如果子进程意外终止, 它返回一个非0值
WSTOPSIG(stat_val);// 如果WIFSTOPED非0, 它返回一个信号值
```

对于多进程程序而言, 父进程一般需要跟踪子进程的退出状态. 因此, 当子进程结束运行是, 内核不会立即释放该进程的进程表表项, 以满足父进程后续对孩子进程推出信息的查询
- 在`子进程结束运行之后, 父进程读取其退出状态前`, 我们称该子进程处于`僵尸态`
- 另外一使子进程进入僵尸态的情况 - 父进程结束或者异常终止, 而子进程继续运行. (子进程的PPID设置为1,init进程接管了子进程) `父进程结束运行之后, 子进程退出之前`, 处于`僵尸态`

以上两种状态都是父进程没有正确处理子进程的返回信息, 子进程都停留在僵尸态, 占据着内核资源.

waitpid()虽然为非阻塞, 则需要在 waitpid所监视的进程结束后再调用.
SIGCHLD信号- 子进程结束后将会给父进程发送此信号
```c++
static void handle_child(int sig)
{
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
	{
		// 善后处理emmmm
	}
}
```

## 信号量-进程的锁
*信号量原语*
只支持两种操作, 等待(wait)和信号(signal) , 在LInux中等待和信号有特殊的含义, 所以又称为P(passeren, 传递就好像进入临界区)V(vrijgeven, 释放就好像退出临界区)操作.
假设有信号量SV(可取任何自然数, 这本书只讨论二进制信号量), 对它的PV操作含义为
- P(SV), 如果SV的值大于0, 就将它减1, 如果sv的值为0 则挂起进程的执行
- V(SV), 如果其他进程因为等待SV而挂起, 则唤醒之, 如果没有则将SV加1
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E4%BD%BF%E7%94%A8%E4%BF%A1%E5%8F%B7%E9%87%8F%E4%BF%9D%E6%8A%A4%E5%85%B3%E9%94%AE%E4%BB%A3%E7%A0%81%E6%AE%B5.png)

**总结PV使用方法**

使用`semget`获取到唯一的标识.
使用`semctl`的`SETVAL`传入初始化val的`sem_un`联合体.来初始化val
调用`semop` 传入唯一标识, `sem_op=-1`执行P(锁)操作`sem_op=1`执行V(开锁)操作
开关锁通过当`sem_op=-1,semval=0 `
且未指定`IPC_NOWAIT`
等待`semval`被`sem_op=1`改为`semval=1`

**创建信号量**
```c++
// semeget 系统调用
// 创建一个全局唯一的信号量集, 或者获取一个已经存在的信号量集
// key 参数是一个键值, 用来标识一个全局唯一的信号量级,可以在不同进程中获取
// num_sems 参数指定要创建/获取的信号量集中信号量的数目. 如果是创建信号量-必须指定, 如果是获取-可以指定为0. 一般都是为1
// sem_flags指定一组标志, 来控制权限
// - 可以与IPC_CREAT 做或运算创建新的信号量集, 即使信号量集存在也不会报错
// - IPC_CREAT | IPC_EXCL来创建一组唯一信号量集 如果已经存在则会返回错误 errno = EEXIST
// 成功返回一个正整数, 是信号量集的标识符, 失败返回 -1
int semget(key_t key, int num_sems, int sem_flags);

int sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
```

**初始化**
```c++
// semctl 系统调用
// sem_id 参数是由semget返回的信号量集标识符
// sen_num指定被操作的信号量在信号集中的编号
// command指定命令, 可以追加命令所需的参数, 不过有推荐格式
// 成功返回对应command的参数, 失败返回-1 errno
int semctl(int sem_id, int sem_num, int command, ...);

// 第四个参数 竟然需要手动声明...
union semun
{
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};
// 初始化信号量
union semun sem_union;
sem_union.val = 1;
// 这里可以直接第三个参数传入1(val)
if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
{
	exit(0);
}

// 删除信号量
union semun sem_union{};
if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
{
	exit(EXIT_FAILURE);
}
```
---
与semop信号量关联的一些重要的内核变量
```c++
unsigned short semval; // 信号量的值
unsigned short semzcnt; // 等待信号量值变为0的进程数量
unsigned short semncnt// 等待信号量值增加的进程数量
pid_t sempid; // 最后一次执行semop操作的进程ID
```
操作信号量, 实际上就是对上面的内核变量操作

```c++
// sem_id 是由semget调用返回的信号量集的标识符, 用以指定被操作的,目标信号量集.
// sem_ops 参数指向一个sembuf结构体类型的数组
// num_sem_ops 说明操作数组中哪个信号量
// 成功返回0, 失败返回-1 errno. 失败的时候sem_ops[] 中的所有操作不执行
int semop(int sem_id, struct sembuf* sem_ops, size_t num_sem_ops);

// sem_op < 0 期望获得信号量
// semval-=abs(sem_op),要求调用进程对被操作信号量集有写权限
// 如果semval的值大于等于sem_op的绝对值, 则操作成功, 调用进程立即获得信号量

// 如果semval < abs(sem_op) 则在被指定IPC_NOWAIT的时候semop立即返回error, errno=EAGIN
// 如果没有指定 则 阻塞进程等待信号量可用, 且 semzcnt +=1, 等到下面三种情况唤醒
// 1 发生semval >= abs(sem_op), semzcnt-=1, semval-=abs(sem_op). 在SEM_UNDO设置时更新semadj
// 2 被操作的信号量所在的信号量集被进程移除, 此时semop调用失败返回, errno=EIDRM (同 sem_op = 0)
// 3 调用被系统中断, 此时semop调用失败返回, errno=EINTR, 同时将该信号量的semzcnt减1 (同 sem_op = 0)
bool P(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0; // 信号量编号 第几个信号量 一般都是第0个
    sem_b.sem_op = -1; // P
	// IPC_NOWAIT 无论信号量操作是否成功, 都立即返回
	// SEM_UNDO当进程退出的时候, 取消正在进行的semop操作 PV操作系统更新进程的semadj变量
    sem_b.sem_flg = SEM_UNDO;
    return semop(sem_id, &sem_b, 1) != -1;
}


// sem_op > 0 
// semval+=sem_op , 要求调用进程对被操作的信号量集有写权限
// 如果此时设置了SEM_UNDO标志, 则系统将更新进程的semadj变量(用以跟踪进程对信号量的修改情况)
bool V(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; // V
    sem_b.sem_flg = SEM_UNDO;
    return semop(sem_id, &sem_b, 1) != -1;
}


// -- sem_op = 0
// -- 标着这是一个`等待0`的操作, 要求调用进程对被操作信号量集有用读权限
// -- 如果此时信号量的值是0, 则调用立即返回, 否则semop失败返回, 或者阻塞进程以等待信号量变为0
// -- 此时如果IPC_NOWAIT 标志被设置, sem_op立即返回错误 errno=EAGAIN
// -- 如果未指定此标志, 则信号量的semzcnt的值增加1, 这时进程被投入睡眠直到下列三个条件之一发生
// -- 1 信号量的值samval变为0, 此时系统将该信号量的semzcnt减1
// -- 2 被操作的信号量所在的信号量集被进程移除, 此时semop调用失败返回, errno=EIDRM
// -- 3 调用被系统中断, 此时semop调用失败返回, errno=EINTR, 同时将该信号量的semzcnt减1

```

semget成功时返回一个与之关联的内核结构体semid_ds
```c++
struct semid_ds
{
	struct ipc_perm sem_perm;
	unsigned long int sem_nsems; // 被设置为num_sems
	time_t sem_otime; // 被设置为0
	time_t sem_ctime; // 被设置为当前的系统时间
}
// 用来描述权限
struct ipc_perm
{
	uid_t uid; // 所有者的有效用户ID, 被semget设置为调用进程的有效用户ID
	gid_t gid; // 所有者的有效组ID, 被semget设置为调用进程的有效用户ID
	uid_t cuid; // 创建者的有效用户ID, 被semget设置为调用进程的有效用户ID
	gid_t cgid; // 创建者的有效组ID, 被semget设置为调用进程的有效用户ID
	mode_t mode;// 访问权限, 背着只为sem_flags参数的最低9位.
}
```

## 共享内存-进程间通信
**最高效的IPC(进程间通信)机制**
需要自己同步进程对其的访问, 否则会产生竞态条件

```c++
// key
// 与semget相同 标识一段全局唯一的共享内存
// size 内存区域大小 单位字节
// shmflg
// IPC_CREAT 存不存在都创建新的共享内存
// IPC_CREAT | IPC_EXCL 不存在则创建 存在则报错
// SHM_HUGETLB 系统将使用"大页面"来为共享内存分配空间
// SHM_NORESERVE 不为共享内存保留swap空间, 如果物理内存不足
// -在执行写操作的时候将会触发`SIGSEGV`信号
// -成功返回唯一标识, 失败返回-1 errno
int shmget(key_t key, size_t size, int shmflg)
```

```c++
// shm_id 
// shmget返回的唯一标识
// shm_addr 
// 关联到进程的哪块地址空间, 其效果还受到shmflg的可选标识SHM_RND的影响
// 如果shm_addr = NULL, 则关联地址由操作系统决定, 代码可移植性强
// 如果 shm_addr 非空,且没有`SHM_RND`标志 则关联到指定的地址处
// 如果 shm_addr 非空, 但是设置了标志 *这里还没用到, 暂时不写*
// shmflg
// SHM_RDONLY 设置后内存内容变成只读, 不设置则为读写模式
// SHM_REMAP 如果地址shmaddr已经关联到一段内存上则重新关联
// SHM_EXEC 有执行权限
// 成功返回关联到的地址, 失败返回 (void*)-1 errno
void* shmat(int shm_id, const void* shm_addr, int shmflg)

// 将共享内存关联到进程的地址空间 调用成功之后, 修改shmid_ds的部分内容
// -shm_nattach +1
// -更新 shm_lpid
// -shm_atime设置为当前时间
```


```c++
// 将共享内存从进程地址空间中分离
// 成功后
// -shm_nattach -1
// -更新 shm_lpid和shm_dtime设置为当前时间
// 成功返回0 失败返回-1 errno
int shmdt(const void* shm_addr)
```

```c++
int shm_ctl(int shm_id, int command, struct shmid_ds* buf)
```
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/shmctl.png)

----

shmget 同时会创建对应的`shmid_ds`结构体
```c++
struct shmid_ds
{
	struct ipc_perm shm_per; // 权限相关
	size_t shm_segsz; // 共享内存大小 单位字节	size
	__time_t shm_atime; // 对这段内存最后一次调用semat的时间 0
	__time_t shm_dtime; // 对这段内存最后一次调用semdt的时间 0
	__time_t shm_ctime; // 对这段内存最后一次调用semctl的时间 当前时间
	__pid_t shm_cpid; // 创建者PID
	__pid_t lpid; // 最后一次执行shmat或shmdt的进程PID
	shmatt_t shm_nattach // 关联到此共享内存空间的进程数量
}
```

**共享内存的POSIX方法**
```c++
int shmfd = shm_open("/shm_name", O_CREAT | O_RDWR, 0666);
ERROR_IF(shmfd == -1, "shm open");

int ret = ftruncate(shmfd, BUFFER_SIZE);
ERROR_IF(ret == -1, "ftruncate");

share_mem = (char*)mmap(nullptr, BUFFER_SIZE,
		PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
ERROR_IF(share_mem == MAP_FAILED, "share_mem");
close(shmfd);

// 取消关联
munmap((void*)share_mem, BUFFER_SIZE);
```

## 进程通信-管道

管道可以在父,子进程间传递数据, 利用的是fork调用后两个文件描述符(fd[0]和fd[1])都保持打开. 一对这样的文件描述符只能保证
父,子进程间一个方向的数据传输, 父进程和子进程必须有一个关闭fd[0], 另一个关闭fd[1].

可以用两个管道来实现双向传输数据, 也可以用`socketpair`来创建管道

## 消息队列
消息队列是两个进程之间传递二进制块数据的一种简单有效的方式.
每个数据块都有自己的类型, 接收方可以根据类型有选择的接收数据

```c++
#include <sys/msg.h>
// 与semget 相同, 成功返回标识符
// msgflg的设置和作用域setget相同
int msgget(key_t key, int msgflg);
```

```c++
// msg_ptr参数指向一个准备发送的消息, 消息必须按如下定义
// msg_sz 指的是mtext的长度!!!
// msgflg通常仅支持IPC_NOWAIT 以非阻塞形式发送数据
int msgsnd(int msqid, const void *msg_ptr, size_t msg_sz, int msgflg);
默认如果消息队列已满, 则会阻塞. 如果设置了 IPC_NOTWAIT
就立即返回 设置errno=EAGIN

系统自带这个结构体 不过mtext长度是1...
struct msgbuf
{
	long mtype; /* 消息类型 正整数*/
	char mtext[512]; /* 消息数据*/
}
```

```c++
// msgtype = 0 读取消息队列第一个消息
// msgtype > 0 读取消息队列第一个类型是msgtype的消息 除非标志了MSG_EXCEPT
// msgtype < 0 读取第一个 类型值 < abs(msgtype)的消息

// IPC_NOWAIT 如果消息队列没有消息, 则msgrcv立即返回并设置errno=ENOMSG
// MSG_EXCEPT 如果msgtype大于0, 则接收第一个非 msgtype 的数据
// MSG_NOERROR 消息部分长度超过msg_sz 则将它截断
int msgrcv(int msqid, void *msg_ptr, size_t msg_sz, long int msgtype, int msgflg);
处于阻塞状态 当消息队列被移除(errno=EIDRM)或者程序接受到信号(errno=EINTR) 都会中断阻塞状态
```

```c++
int msgctl(int msqid, int command, struct msqid_ds *buf);

IPC_STAT 复制消息队列关联的数据结构
IPC_SET 将buf中的部分成员更新到目标的内核数据
IPC_RMID 立即移除消息队列, 唤醒所有等待读消息和写消息的进程
IPC_INFO 获取系统消息队列资源配置信息

MSG_INFO 返回已经分配的消息队列所占用资源信息
MSG_STAT msgqid不再是标识符, 而是内核消息队列的数组索引
```


## 在进程间传递文件描述符

## IPC命令-查看进程间通信的全局唯一key

# 第十四章 多线程编程
根据运行环境和调度者身份, 线程可以分为两种
内核线程
运行在内核空间, 由内核来调度.
用户线程
运行在用空间, 由线程库来调用

当内核线程获得CPU的使用权的时候, 他就加载并运行一个用户线程, 所以内核线程相当于用户线程的容器.

线程有三种实现方式
- 完全在用户空间实现-无需内核支持
	创建和调度线程无需内核干预, 速度很快.
	不占用额外的内核资源, 对系统影响较小
	但是无法运行在多个处理器上, 因为这些用户线程是是实现在一个内核线程上的
- 完全由内核调度
	创建和调度线程的任务都交给了内核, 运行在用户空间的线程库无需管理
	优缺点正好与上一个相反
- 双层调度
	结合了前两个的优点
	不会消耗过多的内核资源,而且线程切换快, 同时它可以充分利用多处理器的优势

## 进程的创建和终止
```c++
#include <pthread.h>
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
// 成功返回0 失败返回错误码
// thread 用来唯一的标识一个新线程
// attr用来设置新县城的属性 传递NULL表示默认线程属性
// start_routine 指定新线程运行的函数
// arg指定函数的参数
```
```c++
void pthread_exit(void* retval);
用来保证线程安全干净的退出, 线程函数最好结束时调用.
通过`retval`参数向线程的回收者传递其退出信息
执行后不会返回到调用者, 而且永远不会失败

int pthread_join(pthread_t thread, void** retval)
可以调用这个函数来回收其他线程 不过线程必须是可回收的该函数会一直阻塞知道被回收的线程结束.
成功时返回0, 失败返回错误码
等待其他线程结束
thread 线程标识符
retval 目标线程的退出返回信息

错误码如下
`EDEADLK`引起死锁, 两个线程互相针对对方调用pthread_join 或者对自身调用
`EINVAL`目标线程是不可回收的, 或是其他线程在回收目标线程
`ESRCH`目标线程不存在

int pthread_cancel(pthread_t thread)
异常终止一个线程, 即为取消线程
成功返回0, 失败返回错误码
```

**线程属性设置**
```c++
接收到取消请求的目标线程可以决定是否允许被取消以及如何取消.
// 启动线程取消
int pthread_setcancelstart(int state, int* oldstate)
第一个参数
PTHREAD_CANCEL_ENABLE 允许线程被取消, 默认状态
PTHREAD_CANCEL_DISABLE 不允许被取消, 如果这种线程接收到取消请求, 则会挂起请求直到
这个线程允许被取消
第二个参数 返回之前设定的状态

// 设置线程取消类型
int pthread_setcanceltype(int type, int* oldtype)
第一个参数
PTHREAD_CANCEL_ASYNCHRONOUS 线程可以随时被取消
PTHREAD_CANCEL_DEFERRED 允许目标现成推迟行动, 直到调用了下面几个所谓的取消点函数
最好使用pthread_testcancel函数设置取消点
设置取消类型(如何取消)
第二个参数
原来的取消类型
```

**设置脱离线程**
```c++
// 初始化线程属性对象
int pthread_attr_init(pthread_attr_t *attr);
// 销毁线程属性对象, 直到再次初始化前都不能用
int pthread_attr_destory(pthread_attr_t *attr)

// 参数取值
// -PTHREAD_CREATE_JOINABLE 线程可回收
// -PTHREAD_CREATE_DETACH 脱离与进程中其他线程的同步 成为脱离线程
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
// 可以直接设置为脱离线程
int pthread_detach(pthread_t thread)
```

## 线程同步机制的使用场景
POSIX信号量-需要自己维护计数值, 用户空间有计数值 信号量自己又计数值
两份计数值容易出错

互斥锁-对临界资源的独占式访问

条件变量-等待某个条件满足
当某个共享数据达到某个值的时候, 唤醒等待这个共享数据的线程

读写锁-可以多个进程读, 读的时候不能写, 同时只能一个写

自旋锁-通过while循环频繁尝试获取锁, 适用于锁事件短, 需要快速切换的场景

## POSIX信号量
多线程也必须考虑线程同步的问题.
虽然`pthread_join()`可以看做简单的线程同步方式不过它无法高效的实现复杂的同步需求
比如无法实现共享资源独占式访问, 或者在某种条件下唤醒指定的指定线程.

```c++
#include<semaphore>
// 用于初始化一个未命名的信号量.
// pshared==0 则表示是当前进程的局部信号量, 否则信号量可以在多个进程间共享
// value指定参数的初始值
int sem_init(sem_t* sem, int pshared, unsigned int value)

// 销毁信号量, 释放其占用的系统资源
int sem_destory(sem_t* sem)

// 以原子操作的形式将信号量的值 -1, 如果信号量的值为0, 则sem_wait将被阻塞直到sem_wait具有非0值
int sem_wait(sem_t* sem)

// 跟上面的函数相同不过不会阻塞. 信号量不为0则减一操作, 为0则返回-1 errno
int sem_trywait(sem_t* sem)

// 原子操作将信号量的值 +1
int sem_post(sem_t* sem)
```
初始化已经存在的信号量会导致无法预期的结果

销毁正被其他线程等待的信号量, 将会导致无法预期的结果

例子如下
```c++
constexpr int kNumberMax = 10;
std::vector<int> number(kNumberMax);

constexpr int kThreadNum = 10;
sem_t sems[kThreadNum];
pthread_t threads[kThreadNum];

constexpr int kPrintTime = 1;

void* t(void *no)
{
    int start_sub = *static_cast<int*>(no);
    int sub =start_sub;
    int time = 0;
    while(++time <= kPrintTime)
    {
		// 锁住本线程 释放下一个线程
        sem_wait(&sems[start_sub]);
        printf("%d\n", number[sub]);
        sem_post(&sems[(start_sub + 1) % kThreadNum]);
		// 计算下一次要打印的下标
        sub = (sub + kThreadNum) % kNumberMax;
    }
    pthread_exit(nullptr);
}

int main()
{
    std::iota(number.begin(), number.end(), 0);
    sem_init(&sems[0], 0, 1);
    for (int i = 1; i < kThreadNum; ++i)
    {
        sem_init(&sems[i], 0, 0);
    }
    for (int i = 0; i < kThreadNum; ++i)
    {
        pthread_create(&threads[i], nullptr, t, &number[i]);
    }
	// 等待最后一个线程结束
    pthread_join(threads[kThreadNum - 1], nullptr);
}
```
kThreadNum个进程依次打印`[0, kNumberMax)`
每个进程打印kPrintTime次
最后一个进程打印完后主线程才能结束

## 互斥锁
```c++
// 初始化互斥锁
// 第一个参数指向目标互斥锁, 第二个参数指定属性 nullptr则为默认
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

// 销毁目标互斥锁
int pthread_mutex_destory(pthread_mutex_t *mutex);

// 针对普通锁加锁
int pthread_mutex_lock(pthread_mutex_t *mutex);

// 针对普通锁立即返回 目标未加锁则加锁 如果已经加锁则返回错误码EBUSY
int pthread_mutex_trylock(pthread_mutex_t *mutex);

// 解锁 如果有其他线程在等待这个互斥锁, 则其中之一获得
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

销毁一个已经加锁的互斥锁 会发生不可预期的后果
也可使使用宏`PTHREAD_MUTEX_INITIALIZER`来初始化一个互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


**互斥锁属性设置**

```c++
int pthread_mutexattr_init(pthread_mutexattr_t *attr);

int pthread_mutexattr_destory(pthread_mutexattr_t *attr);

// PTHREAD_PROCESS_SHARED 跨进程共享
// PTHREAD_PROCESS_PRIVATE 隶属同一进程的线程
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared);
int pthread_mutexattr_setpshared(const pthread_mutexattr_t *attr, int pshared);

// PTHREAD_MUTEX_NORMAL 普通锁 默认类型
// PTHREAD_MUTEX_ERRORCHECK 检错锁
// PTHREAD_MUTEX_RECURSVE 嵌套锁
// PTHREAD_MUTEX_DEFAULT 默认锁
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutexattr_settype(const pthread_mutexattr_t *attr, int type);
```
PTHREAD_MUTEX_NORMAL
一个线程对其加锁后, 其他请求该锁的进程会形成一个等待队列, 解锁后然后按照优先级获得. 保证资源分配公平
A线程对一个`已经加锁`的普通锁`再次加锁(也是A线程)`-同一线程在解锁前再次加锁引发死锁
对一个已经`被其他线程加锁`的普通锁`解锁`, 或者`再次解锁已经解锁`的普通锁--解锁-不可预期后果

PTHREAD_MUTEX_ERRORCHECK
线程对`已经加锁`的检错锁`再次加锁`--加锁-加锁操作返回EDEADLK
对一个已经`被其他线程加锁`的检错锁`解锁`, 或者`再次解锁已经解锁`的检错锁--解锁-返回EPERM

PTHREAD_MUTEX_RECURSVE
允许一个线程在释放锁前多次加锁 而不发生死锁.
如果`其他线程`要获得这个锁, 则`当前锁拥有者`必须执行相应次数的解锁操作--加锁
对于`已经被其他进程`加锁的嵌套锁解锁, 或者对`已经解锁`的再次解锁--解锁-返回EPERM

PTHREAD_MUTEX_DEFAULT
这种锁的实现可能为上面三种之一
对已经加锁的默认锁再次加锁
对被其他线程加锁的默认锁解锁
再次解锁已经解锁的默认锁
都将会发生不可预料后果

例子
```c++
pthread_mutex_t mutex;
int count = 0;
void* t(void *a)
{
    pthread_mutex_lock(&mutex);
    printf("%d\n", count);
    count++;
    pthread_mutex_unlock(&mutex);
}
int main()
{
    pthread_mutex_init(&mutex, nullptr);
    pthread_t thread[10];
    for (int i = 0; i < 10; ++i)
    {
        pthread_create(&thread[i], nullptr, t, nullptr);
    }
    sleep(3);
    pthread_mutex_destroy(&mutex);
}
```

## 条件变量

```c++
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr *cond_attr);

// 销毁一个正在被等待的条件变量 将会失败并返回EBUSY
int pthread_cont_destory(pthread_cond_t *cond);

// 广播式的唤醒所有等待目标条件变量的线程
int pthread_cont_broadcast(pthread_cond_t *cond);

// 唤醒一个等待目标条件变量的线程
int pthread_cond_signal(pthread_cond_t *cond);

// 等待目标条件变量
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
```

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
将各个字段初始化为0

pthread_cond_wait的第二个参数, 用于保护条件变量的互斥锁
掉用函数前必须将 mutex加锁, 否则会发生不可预料的后果.
函数执行前将调用线程放入条件变量等待队列, 然后将mutex解锁

从函数调用, 到被放入等待队列的时间内, pthread_cond_signal(broadcast)不会修改条件变量的值
也就是 pthread_cond_wait函数不会错过目标条件变量的任何变化,
将pthread_cond_wait函数返回的时候, 互斥锁mutex将会再次锁上

例子
```c++
pthread_mutex_t mutex;
pthread_cond_t cond;
int good = 3;
int produce_count = 0;
int consume_count = 0;

void* Producer(void *arg)
{
    while(produce_count < 10)
    {
        pthread_mutex_lock(&mutex);
        good++;
        pthread_mutex_unlock(&mutex);

        produce_count++;
        printf("produce a good\n");
		// 通知一个线程
        pthread_cond_signal(&cond);
        sleep(2);
    }
    pthread_exit(nullptr);
}

void* Consumer(void *arg)
{
    while (consume_count < 13)
    {
		// 传入前需要加锁
        pthread_mutex_lock(&mutex);
        if (good > 0)
        {
            good--;
            consume_count++;
            printf("consume a good, reset %d\n", good);
        }
        else
        {
            printf("good is 0\n");
            // wait pthread_cond_signal
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        usleep(500 * 1000);
    }
    pthread_exit(nullptr);
}

int main()
{
    mutex = PTHREAD_MUTEX_INITIALIZER;
    cond = PTHREAD_COND_INITIALIZER;
    pthread_t producer, consumer;

    pthread_create(&consumer, nullptr, Consumer, nullptr);
    pthread_create(&producer, nullptr, Producer, nullptr);

    pthread_join(consumer, nullptr);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}
```

## 读写锁

## 自旋锁


## 线程同步包装类-多线程环境
```c++
class Sem
{
public:
    Sem()
    {
        if (sem_init(&sem_, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    ~Sem()
    {
        sem_destroy(&sem_);
    }
    bool Wait()
    {
        return sem_wait(&sem_) == 0;
    }
    bool Post()
    {
        return sem_post(&sem_) == 0;
    }
private:
    sem_t sem_;
};

class Mutex
{
public:
    Mutex()
    {
        if (pthread_mutex_init(&mutex_, nullptr) != 0)
        {
            throw std::exception();
        }

    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }
    bool Lock()
    {
        return pthread_mutex_lock(&mutex_) == 0;
    }
    bool Unlock()
    {
        return pthread_mutex_unlock(&mutex_) == 0;
    }

private:
    pthread_mutex_t mutex_;
};

class Cond
{
public:
    Cond()
    {
        if (pthread_mutex_init(&mutex_, nullptr) != 0)
        {
            throw std::exception();
        }
        if (pthread_cond_init(&cond_, nullptr) != 0)
        {
            // 这里我一开始没有想到..
            pthread_mutex_destroy(&mutex_);
            throw std::exception();
        }
    }
    ~Cond()
    {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cond_);
    };
    bool Wait()
    {
        int ret = 0;
        pthread_mutex_lock(&mutex_);
        ret = pthread_cond_wait(&cond_, &mutex_);
        pthread_mutex_unlock(&mutex_);
        return ret == 0;
    }
    bool Signal()
    {
        return pthread_cond_signal(&cond_) == 0;
    }
private:
    pthread_cond_t cond_;
    pthread_mutex_t mutex_;
};
```

线程安全或可重入函数--函数能被多个线程同时调用而不发生竞态条件

多线程程序某个线程调用fork函数, 新进程不会与父进程有相同数量的线程
子进程只有一个线程-调用fork线程的完美复制

但是子进程会继承父进程的互斥锁(条件变量)的状态, 如果互斥锁被加锁了, 但`不是由`调用fork线程
锁住的, 此时`子进程`再次对这个互斥锁`执行加锁`操作将会`死锁`.

```c++
pthread_mutex_t mutex;
void* another(void *arg)
{
    printf("in child thread, lock the mutex\n");
    pthread_mutex_lock(&mutex);
    sleep(5);
    // 解锁后 Prepare才能加锁
    pthread_mutex_unlock(&mutex);
    pthread_exit(nullptr);
}
// 这个函数在fork创建子进程前被调用
void Prepare()
{
    // 但是会阻塞 直到执行another函数的线程解锁 才能够继续执行
    // 这个函数执行完毕前fork不会创建子进程
    pthread_mutex_lock(&mutex);
}
// fork创建线程后 返回前 会在子进程和父进程中执行这个函数
void Infork()
{
    pthread_mutex_unlock(&mutex);
}
int main()
{
    pthread_mutex_init(&mutex, nullptr);
    pthread_t id;
    pthread_create(&id, nullptr, another, nullptr);

    sleep(1);
    // pthread_atfork(Prepare, Infork, Infork);
    int pid = fork();
    if (pid < 0)
    {
        printf("emmm????\n");
        pthread_join(id, nullptr);
        pthread_mutex_destroy(&mutex);
        return 1;
    }
    else if (pid == 0)
    {
        printf("child process, want to get the lock\n");
        pthread_mutex_lock(&mutex);
        printf("i cann't run to here, opps....\n");
        pthread_mutex_unlock(&mutex);
        exit(0);
    }
    else
    {
        printf("wait start\n");
        wait(nullptr);
        printf("wait over\n"); // 没有打印 因为子进程不会终止
    }
    pthread_join(id, nullptr);
    pthread_mutex_destroy(&mutex);
    return 0;
}
// $ in child thread, lock the mutex
// $ wait start
// $ child process, want to get the lock

// $ in child thread, lock the mutex
// $ wait start
// $ child process, want to get the lock
// $ i cann't run to here, opps....
// $ wait over
```

原版就会发生死锁, 新版(去掉注释的代码) 能够正常运行

```c++
int pthread_atfork (void (*__prepare) (void),
			   void (*__parent) (void),
			   void (*__child) (void));
```
第一个句柄 在fork创建子进程前执行
第二个句柄 在fork创建出子进程后, fork返回前在父进程中执行
第二个句柄 在fork创建出子进程后, fork返回前在子进程中执行


# 第十五章 进程池和线程池

# 线程池 和 简单HTTP服务器
对我而言神秘已久的线程池终于揭开了面纱.
没想到这就是线程池23333

线程池写完后 直接写了书上的HTTP服务器.

那个服务器至少我发现两个问题
- 无法发送大文件
- 部分请求无法回复

无法发送大文件, 是因为书中使用了writev发送数据
期初我以为下面的判断 writev返回值 等于 -1就是为了发送大文件, 后来发现这个判断只是给期初就发送失败准备的.

正好前一阵子看了一个服务器的代码
https://github.com/Jigokubana/Notes-flamingo


我就索性直接将发送部分修改了
```c++
// write_sum_ 需发送总大小
// write_idx_ 已发送大小

int temp = 0;
if (write_sum_ - write_idx_ == 0)
{
    Modfd(epollfd_, sockfd_, EPOLLIN);
    Init();
    return true;
}
while (true)
{
    temp = send(sockfd_, &*write_buff_.begin() + write_idx_, write_sum_ - write_idx_, 0);
    if (temp <= -1)
    {
        if (errno == EAGAIN)
        {
            Modfd(epollfd_, sockfd_, EPOLLOUT);
            return true;
        }
    }
    write_idx_ += temp;

    if (write_idx_ == write_sum_)
    {
        // 解除绑定移到了其他地方
        if (linger_)
        {
            Init();
            Modfd(epollfd_, sockfd_, EPOLLIN);
            return true;
        }
        else
        {
            Modfd(epollfd_, sockfd_, EPOLLIN);
            return false;
        }
    }
}
```

第二个奇葩的问题就是使用ab压测时候 有些请求无法收到回复.
这个问题等后面在解决把, 等我知识更加丰富了再说