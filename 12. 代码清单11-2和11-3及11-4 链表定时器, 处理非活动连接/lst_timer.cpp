//
// Created by lsmg on 2/1/20.
//

#include <time.h>
#include <stddef.h>
#include <cstdio>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

class util_timer;

// 用户数据结构 客户端的socket地址 socket文件描述符 读缓存 定时器
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

class util_timer
{
public:
    util_timer():prev(nullptr), next(nullptr){};
public:
    util_timer *prev; // 前驱
    util_timer *next; // 后继

    time_t expire; // 超时时间
    client_data *user_data; // 回调函数处理的客户端数据
    void (*cb_func)(client_data*); // 回调函数
};

// 定时器链表 升续 双向 带有头尾节点
class sort_timer_lst
{
public:
    sort_timer_lst():head(nullptr), tail(nullptr){};
    // 析构时删除其中所有的定时器
    ~sort_timer_lst()
    {
        util_timer *temp = head;
        while(nullptr != temp)
        {
            delete head;
            head = temp->next;
            temp = head;
        }
    }
    // 将目标定时器添加到链表中
    void add_timer(util_timer *timer)
    {
        if (nullptr == timer)
        {
            return;
        }
        if (nullptr == head)
        {
            head = timer;
            tail = timer;
            return;
        }
        // 超时时间小于链表中最小时间 直接插入头部
        if (timer->expire < head->expire)
        {
            head = timer;
            timer->next = tail;
            tail->prev = timer;
            return;
        }
        // 需要插入到链表中 或链表尾部
        add_timer(timer, head);
    }

    // 增加timer定时器expire
    void adjust_timer(util_timer *timer)
    {
        if (nullptr == timer)
        {
            return;
        }
        util_timer *temp = timer->next;
        if (timer == tail || (timer->expire < temp->expire))
        {
            return;
        }
        // 如果调整的定时器 是head 则需要取出这个定时器
        if (timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            timer->next = nullptr;
            add_timer(timer, head);
        }
        else
        {
            // 不是head 在中间部分
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }

    void del_timer(util_timer *timer)
    {
        if (nullptr == timer)
        {
            return;
        }
        if ((timer ==  head) && (timer == tail))
        {
            delete timer;
            head = nullptr;
            tail = nullptr;
            return;
        }
        if (timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            delete timer;
            return;
        }
        if (timer == tail)
        {
            tail = tail->prev;
            tail->next = nullptr;
            delete timer;
            return;
        }
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    void tick()
    {
        if (nullptr == head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(NULL);
        util_timer *temp = head;
        while (nullptr != head)
        {
            if (cur < temp->expire)
            {
                break;
            }
            temp->cb_func(temp->user_data);
            head = temp->next;
            if (nullptr != head)
            {
                head->prev = nullptr;
            }
            delete temp;
            temp = head;
        }
    }
private:
    util_timer *head;
    util_timer *tail;

private:
    void add_timer(util_timer *timer, util_timer *lst_head)
    {
        util_timer *temp = lst_head;
        // 更新temp节点为timer->expire < temp->expire
        while (timer->expire > temp->expire)
        {
            if (temp->next != nullptr)
            {
                temp = temp->next;
            }
            else
            {
                // 如果temp == tail 并且expire依然大 则说明应该插入尾部
                tail->next = timer;
                timer->prev = tail;
                tail = timer;
                return;
            }
        }

        timer->next = temp;
        timer->prev = temp->prev;
        temp->prev->next = timer;
        temp->prev = timer;
    }
};