//
// Created by lsmg on 3/24/20.
//

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <sys/mman.h>
#include <sys/uio.h>
#include <cstdarg>
#include "httpconnection.h"

const char * const OK_200_TITILE = "OK";
const char * const ERROR_400_TITILE = "Bad Request";
const char * const ERROR_400_FORM = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char * const ERROR_403_TITLE = "Forbidden";
const char * const ERROR_403_FORM = "You don't have the permission to get file from this server.\n";
const char * const ERROR_404_TITLE = "Not Fount";
const char * const ERROR_404_FORM = "The requested file was not found on this server.\n";
const char * const ERROR_500_TITLE = "Internal Error";
const char * const ERROR_500_FORM = "There was an unusual problem serving the requested file.\n";

// website roor dir
const char * const DOC_ROOT = "/home/lsmg/web";

int SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Addfd(int epollfd, int fd, bool one_shot)
{
    struct epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (one_shot)
    {
        // 单个fd只触发一次epollwait返回 之后需要重新注册
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    SetNonblocking(fd);
}

void Removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void Modfd(int epollfd, int fd, int event)
{
    struct epoll_event ev{};
    ev.events = event | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

int HttpConnection::user_count_ = 0;
int HttpConnection::epollfd_ = -1;

HttpConnection::HttpConnection():
write_buff_(DEFAULT_WRITE_BUFFER_SIZE)
{

}

HttpConnection::~HttpConnection()
{

}

void HttpConnection::Init(int sockfd, const sockaddr_in &addr)
{
    sockfd_ = sockfd;
    address_ = addr;

    Addfd(epollfd_, sockfd_, true);
    user_count_++;
    Init();
}

void HttpConnection::CloseConn(bool read_close)
{
    if (read_close && (sockfd_ != -1))
    {
        printf("fd: %d close connection\n", sockfd_);
        Removefd(epollfd_, sockfd_);
        sockfd_ = -1;
        user_count_--;
    }
}

void HttpConnection::Process()
{
    HttpCode read_ret = ProcessRead();
    if (read_ret == kNoRequest)
    {
        Modfd(epollfd_, sockfd_, EPOLLIN);
        return;
    }
    bool write_ret = ProcessWrite(read_ret);
    if (!write_ret)
    {
        CloseConn();
    }
    Modfd(epollfd_, sockfd_, EPOLLOUT);
}

bool HttpConnection::Read()
{
    if (read_idx_ >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;
    while (true)
    {
        bytes_read = recv(sockfd_, read_buff_ + read_idx_, READ_BUFFER_SIZE - read_idx_, 0);

        if (bytes_read == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return false;
        }
        else if (bytes_read == 0)
        {
            return false;
        }
        read_idx_ += bytes_read;
    }
    return true;
}

bool HttpConnection::Write()
{
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
}

void HttpConnection::Init()
{
    check_state_ = kCheckStateRequestLine;
    linger_ = false;
    method_ = kGet;
    url_ = nullptr;
    version_ = nullptr;
    host_ = nullptr;
    content_length_ = 0;

    start_line_ = 0;
    checked_idx_ = 0;
    read_idx_ = 0;
    write_idx_ = 0;
    write_sum_ = 0;
    memset(read_buff_, '\0', READ_BUFFER_SIZE);
    memset(full_file_, '\0', READ_BUFFER_SIZE);
}

HttpConnection::HttpCode HttpConnection::ProcessRead()
{
    LineStatus line_status = kLineOk;
    HttpCode ret = kNoRequest;
    char *text = nullptr;
    while (((check_state_ == kCheckStateContent) && (line_status == kLineOk)) ||
           ((line_status = ParseLine()) == kLineOk))
    {
        text = GetLine();
        start_line_ = checked_idx_;
        switch (check_state_)
        {
            case kCheckStateRequestLine:
            {
                ret = ParseRequestLine(text);
                if (ret == kBadRequest)
                {
                    return kBadRequest;
                }
                break;
            }
            case kCheckStateHeader:
            {
                ret = ParseHeaders(text);
                if (ret == kBadRequest)
                {
                    return kBadRequest;
                }
                if (ret == kGetRequest)
                {
                    return DoRequest();
                }
                break;
            }
            case kCheckStateContent:
            {
                ret = ParseContent(text);
                if (ret == kGetRequest)
                {
                    return DoRequest();
                }
                line_status = kLineOpen;
                break;
            }
            default:
            {
                return kInternalError;
            }
        }
    }
    return kNoRequest;
}

bool HttpConnection::ProcessWrite(HttpConnection::HttpCode ret)
{
    switch (ret)
    {
        case kBadRequest:
        {
           AddStatusLine(400, ERROR_400_TITILE);
           AddHeaders(strlen(ERROR_400_FORM));
           if (!AddContent(ERROR_400_FORM))
           {
               return false;
           }
           break;
        }
        case kNoResource:
        {
            AddStatusLine(404, ERROR_404_TITLE);
            AddHeaders(strlen(ERROR_404_FORM));
            if (!AddContent(ERROR_404_FORM))
            {
                return false;
            }
            break;
        }
        case kForbiddenRequest:
        {
            AddStatusLine(403, ERROR_403_TITLE);
            AddHeaders(strlen(ERROR_403_FORM));
            if (!AddContent(ERROR_403_FORM))
            {
                return false;
            }
            break;
        }
        case kFileRequest:
        {
            AddStatusLine(200, OK_200_TITILE);
            if (file_stat_.st_size != 0)
            {
                AddHeaders(file_stat_.st_size);
                int filesize = file_stat_.st_size;

                write_buff_.resize(DEFAULT_WRITE_BUFFER_SIZE + file_stat_.st_size);
                std::copy(file_address_, file_address_ + filesize, &*write_buff_.begin() + write_sum_);
                write_sum_ += filesize;
                Unmap();
                return true;
            }
            else
            {
                const char * ok_string = "<html><body></body></html>";
                AddHeaders(strlen(ok_string));
                if (!AddContent(ok_string))
                {
                    return false;
                }
            }
        }
        case kInternalError:
        {
            AddStatusLine(500, ERROR_500_TITLE);
            AddHeaders(strlen(ERROR_500_FORM));
            if (!AddContent(ERROR_500_FORM))
            {
                return false;
            }
            break;
        }
        default:
        {
            return false;
        }
    }
    return true;
}

HttpConnection::HttpCode HttpConnection::ParseRequestLine(char *text)
{
    url_ = strpbrk(text, " \t");
    if (!url_)
    {
        return kBadRequest;
    }
    //   去处 空格或者 \t
    *url_++ = '\0';

    char *method = text;
    if (strcasecmp(method, "GET") == 0)
    {
        method_ = kGet;
    }
    else
    {
        return kBadRequest;
    }
    //  又去除一次空格？？
    url_ += strspn(url_, " \t");
    version_ = strpbrk(url_, " \t");
    if (!version_)
    {
        return kBadRequest;
    }
    *version_++ = '\0';
    version_ += strspn(version_, " \t");
    if (strcasecmp(version_, "HTTP/1.1") != 0)
    {
        if (strcasecmp(version_, "HTTP/1.0") != 0)
        {
            return kBadRequest;
        }
        linger_ = false;
    }
    if (strncasecmp(url_, "http://", 7) == 0)
    {
        url_ += 7;
        url_ = strchr(url_, '/');
    }
    if (!url_ || url_[0] != '/')
    {
        return kBadRequest;
    }
    check_state_ = kCheckStateHeader;
    return kNoRequest;
}

HttpConnection::HttpCode HttpConnection::ParseHeaders(char *text)
{
    //  读取到空行 说明头部解析完毕
    if (text[0] == '\0')
    {
        // 如果有消息体 则需要解析消息体
        if (content_length_ != 0)
        {
            check_state_ = kCheckStateContent;
            return kNoRequest;
        }
        return kGetRequest;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            linger_ = true;
        }
    }
    else if (strncasecmp(text, "Content-Length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        content_length_ = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        host_ = text;
    }
    else
    {

    }
    return kNoRequest;
}

HttpConnection::HttpCode HttpConnection::ParseContent(char *text)
{
    if (read_idx_ >= content_length_ + checked_idx_)
    {
        text[content_length_] = '\0';
        return kGetRequest;
    }
    return kNoRequest;
}

HttpConnection::HttpCode HttpConnection::DoRequest()
{
    strcpy(full_file_, DOC_ROOT);
    int len = strlen(DOC_ROOT);
    strncpy(full_file_ + len, url_, FILENAME_MAXLEN - len - 1);
    if (stat(full_file_, &file_stat_) < 0)
    {
        return kNoResource;
    }
    if (!(file_stat_.st_mode & S_IROTH))
    {
        return kForbiddenRequest;
    }
    if (S_ISDIR(file_stat_.st_mode))
    {
        return kBadRequest;
    }
    int fd = open(full_file_, O_RDONLY);
    file_address_ = static_cast<char*>(mmap(nullptr, file_stat_.st_size, PROT_READ,
            MAP_PRIVATE, fd, 0));
    close(fd);

    return kFileRequest;
}

char *HttpConnection::GetLine()
{
    return read_buff_ + start_line_;
}

HttpConnection::LineStatus HttpConnection::ParseLine()
{
    char temp = '\0';
    for (;checked_idx_ < read_idx_; ++checked_idx_)
    {
        temp = read_buff_[checked_idx_];
        if (temp == '\r')
        {
            if (checked_idx_ + 1 == read_idx_)
            {
                return kLineOpen;
            }
            else if (read_buff_[checked_idx_ + 1] == '\n')
            {
                read_buff_[checked_idx_++] = '\0';
                read_buff_[checked_idx_++] = '\0';
                return kLineOk;
            }
            return kLineBad;
        }
        else if (temp == '\n')
        {
            if (checked_idx_ > 1 && read_buff_[checked_idx_ - 1] == '\r')
            {
                read_buff_[checked_idx_ - 1] = '\0';
                read_buff_[checked_idx_++] = '\0';
                return kLineOk;
            }
            return kLineBad;
        }
    }
    return kLineOpen;
}

void HttpConnection::Unmap()
{
    if (file_address_)
    {
        munmap(file_address_, file_stat_.st_size);
        file_address_ = nullptr;
    }
}

bool HttpConnection::AddResponse(const char *format, ...)
{
    if (write_sum_ >= DEFAULT_WRITE_BUFFER_SIZE)
    {
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(&*write_buff_.begin() + write_sum_, DEFAULT_WRITE_BUFFER_SIZE - 1 - write_sum_,
                        format, arg_list);
    if (len >= (DEFAULT_WRITE_BUFFER_SIZE - 1 - write_sum_))
    {
        return false;
    }
    write_sum_ += len;
    va_end(arg_list);
    return true;
}

bool HttpConnection::AddContent(const char *content)
{
    return AddResponse("%s", content);
}

bool HttpConnection::AddStatusLine(int status, const char *title)
{
    return AddResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HttpConnection::AddHeaders(int content_length)
{
    AddContentLength(content_length);
    AddLinger();
    AddBlankLine();
}

bool HttpConnection::AddContentLength(int content_length)
{
    return AddResponse("Content-Length: %d\r\n", content_length);
}

bool HttpConnection::AddLinger() {
    return AddResponse("Connection: %s\r\n", linger_ ? "keep-alive" : "close");
}

bool HttpConnection::AddBlankLine()
{
    return AddResponse("%s", "\r\n");
}
