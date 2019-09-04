---
title: Linux高性能服务器编程记录

---
开头选了这本书, 看到这本书是他人总结三大本后自己写的, 先求个理解大概.
话说别人推荐的c++服务器咋都是c语言.....
2019年8月17日20:37:53

每学习一部分就写一个demo

# demo 记录
```
2019年8月20日 编写demo <1. 客户端自动发送固定信息> -- 所用基础连接部分和TCP连接部分
2019年8月22日 编写demo <2. 伪全双工TCP> -- 所用截止到网络Api之前
2019年8月22日 编写demo <3. 伪全双工UDP> -- 所用截止到网络Api之前
2019年8月25日 复现 源码 <4. 代码清单5-12 访问daytime服务> -- 第五章结束
2019年8月28日 复现 源码 <5. 代码清单6-3 用sendfile函数传输文件>
2019年8月28日 复现 源码 <6. 代码清单6-4 用splice函数实现的回射服务器>
2019年9月02日 编写demo <7. 贪吃蛇客户端及服务器>
2019年9月04日 编写demo <8. 服务器模型-CS模型>
```
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

## 贪吃蛇客户端及服务器
贪吃蛇源码来源于网络, 后续会更换成自己的贪吃蛇程序.

开始编写的服务器, 在socket异常关闭后 会收到大量的空字符串

## 服务器模型-CS模型
这个代码书上没有, 从网络上学习 并进行更改 后续会更改这个demo 实现聊天程序
个人修改部分 为 判断socket为非服务器fd后的操作, 使用了splice函数 回复信息还有 通过splice函数的返回值确认是否此fd已经失效, 需要删除


分了三篇
# 第一篇TCP/IP协议详解
## 第一章 TCP/IP协议族

### TCP/IP协议族体系结构和主要协议
协议族中协议众多, 这本书只选取了IP和TCP协议 - 对网络编程影响最直接

见得最多就是这四层结构了, 不过这本书写得更加详细一些
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%9B%9B%E5%B1%82%E7%BB%93%E6%9E%84.jpg)

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
## 第六章高级IO函数

Linux提供的高级IO函数, 自然是特定条件下能力更强, 不然要他干啥, 特定条件自然限制了他的使用频率

### 创建文件描述符
#### pipe函数
这个函数可用于创建一个管道, 实现进程间的通信. (后面13.4节介绍更多, 待我学到后回来补坑)

```c
// 函数定义
// 参数文件描述符数组 fd[0] 读入 fd[1]写入 单向管道
// 成功返回0, 并将一对打开的文件描述符填入其参数指向的数组
// 失败返回-1 errno
#include <unistd.h>
int pipe(int fd[2]);

// 双向管道
// 第一个参数为 协议PF_UNIX(书上是AF_UNIX, 还需要实际测试下)
#include <sys/types.h>
#include <sys/socket.h>
int socketpair(int domain, int type, int protocol, int fd[2]);
```
#### dup和dup2函数
**文件描述符**
```
文件描述符在是一个非负整数。是一个索引值,指向内核为每一个进程所维护的该进程打开文件的记录表。
STDOUT_FILENO(值为1)- 值为1的文件描述符为标准输出, 关闭STDOUT_FILENO后用dup即可返回最小可用值(目前为, 1) 这样输出就重定向到了调用dup的参数指向的文件
```

将标准输入重定向到一个`文件`或一个网络连接
```c
#include <unistd.h>
// 返回的文件描述符总是取系统当前可用的最小整数值
int dup(int file_descriptor);
// 返回的整数值不小于file_descriptor_two参数.
int dup2(int file_descriptor, int file_descriptor_two);
```
dup函数创建一个新的文件描述符, 新的文件描述符和原有的file_descriptor共同指向相同的目标.

### 读写数据
#### readv/writev
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

int stat(const char *path, struct stat *buf);
// 当路径指向为符号链接的时候, lstat为符号链接的信息. 二stat为符号链接指向文件信息
int lstat(const char *path, struct stat *buf);

/*
* ls -s source dist  建立软连接, 类似快捷方式, 也叫符号链接
* ls source dist  建立硬链接, 同一个文件使用多个不同的别名, 指向同一个文件数据块, 只要硬链接不被完全
* 删除就可以正常访问
* 文件数据块 - 文件的真正数据是一个文件数据块, 打开的`文件`指向这个数据块, 就是说
* `文件`本身就类似快捷方式, 指向文件存在的区域.
*/
```
#### mmap和munmap函数

一个创建一块进程通讯共享的内存(可以将文件映射入其中), 一个释放这块内存
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
#### splice函数
用于在两个文件名描述符之间移动数据, 0拷贝操作
```c
#include <fcntl.h>
// fd_in 为文件描述符, 如果为管道文件描述符则 off_in必须为NULL, 否则为读取开始偏移位置
// len为指定移动的数据长度, flags参数控制数据如何移动.
// - SPLICE_F_NONBLOCK 非阻塞splice操作, 但会受文件描述符自身的阻塞
// - SPLICE_F_MORE 给内核一个提示, 后续的splice调用将读取更多的数据???????
ssize_t splice(int fd_in, loff_t* off_in, int fd_out, loff_t* off_out, size_t len, unsigned int flags);
```
#### select() 函数

```c
#include <fcntl.h> 
// maxfdp 最大数 FD_SETSIZE
// struct fd_set 一个集合,可以存储多个文件描述符
// - FD_ZERO(&fd_set) 清空 -FD_SET(fd, &fd_set) 放入fd FD_CLR(fd, &fd_set)从其中清除fd
// - FD_ISSET(fd, &fd_set) 判断是否在其中
// readfds  需要监视的文件描述符读变化
// writefds 需要监视的文件描述符写变化
// errorfds 错误
// 返回值 负值为错误 0 为超时
int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval*timeout); 
// int result_select = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval*)0);
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

### 服务器模型-CS模型

**优点**
- 实现起来简单
**缺点**
- 服务器是通信的中心, 访问过大的时候会导致响应过慢

模式图
![](https://lsmg-img.oss-cn-beijing.aliyuncs.com/Linux%E9%AB%98%E6%80%A7%E8%83%BD%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%BC%96%E7%A8%8B%E8%AF%BB%E4%B9%A6%E8%AE%B0%E5%BD%95/%E5%9B%BE8-2%20TCP%E6%9C%8D%E5%8A%A1%E5%99%A8%E5%92%8C%E5%AE%A2%E6%88%B7%E7%AB%AF%E5%B7%A5%E4%BD%9C%E6%B5%81%E7%A8%8B.png)

编写的demo 没有用到fork函数. 后续待完善