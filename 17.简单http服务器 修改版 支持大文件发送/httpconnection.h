//
// Created by lsmg on 3/24/20.
//

#ifndef PTHREAD_HTTPCONNECTION_H
#define PTHREAD_HTTPCONNECTION_H

#include <netinet/in.h>
#include <sys/stat.h>
#include <vector>

class HttpConnection
{
public:
    const static int FILENAME_MAXLEN = 200;
    const static int READ_BUFFER_SIZE = 2048;
    const static int DEFAULT_WRITE_BUFFER_SIZE = 1024;

    enum Method {kGet = 0, kPost, kPut, kDelete};

    enum CheckState {kCheckStateRequestLine = 0, kCheckStateHeader, kCheckStateContent};

    enum HttpCode {kNoRequest, kGetRequest, kBadRequest, kNoResource, kForbiddenRequest,
            kFileRequest, kInternalError, kClosedConnection};

    enum LineStatus {kLineOk = 0, kLineBad, kLineOpen};

public:
    HttpConnection();
    ~HttpConnection();

public:
    /**
     * 初始化接受新的连接
     * @param sockfd
     * @param addr
     */
    void Init(int sockfd, const sockaddr_in &addr);

    /**
     * 关闭连接
     * @param read_close
     */
    void CloseConn(bool read_close = true);

    /**
     * 处理请求
     */
    void Process();

    /**
     * 非阻塞读操作
     * @return
     */
    bool Read();

    /**
     * 非阻塞写操作
     * @return
     */
    bool Write();

private:
    /**
     * 初始化连接
     */
    void Init();

    /**
     * 解析Http请求
     * @return
     */
    HttpCode ProcessRead();

    /**
     * 填充HTTP应答
     * @param ret
     * @return
     */
    bool ProcessWrite(HttpCode ret);

    // 用于解析Http请求
    HttpCode ParseRequestLine(char *text);
    HttpCode ParseHeaders(char *text);
    HttpCode ParseContent(char *text);
    HttpCode DoRequest();
    char* GetLine();
    LineStatus ParseLine();

    // 用于填充Http应答
    void Unmap();
    bool AddResponse(const char *format, ...);
    bool AddContent(const char *content);
    bool AddStatusLine(int status, const char *title);
    bool AddHeaders(int content_length);
    bool AddContentLength(int content_length);
    bool AddLinger();
    bool AddBlankLine();

public:
    static int epollfd_;

    // 统计用户数量
    static int user_count_;
private:
    // 该Http连接的socket和对方的socket地址
    int sockfd_;
    sockaddr_in address_;

    char read_buff_[READ_BUFFER_SIZE];
    int read_idx_;
    int checked_idx_;
    int start_line_;

    std::vector<char> write_buff_;
    int write_idx_;
    int write_sum_;

    // 状态机当前状态
    CheckState check_state_;
    Method method_;

    // 请求文件的完整路径名
    char full_file_[FILENAME_MAXLEN];

    // request中的文件名
    char *url_;
    char *version_;
    char *host_;
    int content_length_;
    bool linger_;

    // 请求的目标文件被mmap到内存中的起始位置
    char *file_address_;
    struct stat file_stat_;
};

#endif //PTHREAD_HTTPCONNECTION_H


















