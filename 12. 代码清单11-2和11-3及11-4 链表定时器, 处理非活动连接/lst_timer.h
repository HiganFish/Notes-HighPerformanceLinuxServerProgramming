#ifndef LST_TIMER
#define LST_TIMER

#include <ctime>
#include <cstdio>
#include <netinet/in.h>

#define BUFFER_SIZE 64

class util_timer;

// 用户数据结构 socket地址， 文件描述符， 读缓存 ， 定时器
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
	util_timer() : prev(nullptr), next(nullptr) {}

public:
	time_t expire{}; // 任务超时时间
	void (*cb_func) (client_data*){}; // 任务回调函数
	client_data* user_data{};
	util_timer* prev; // 指向前一个定时器
	util_timer* next; // 指向后一个定时器
};

// 定时器list 生序双向链表 带有头尾节点
class sort_timer_lst
{
public:
	sort_timer_lst() : head(nullptr), tail(nullptr) {}

	~sort_timer_lst()
	{
		util_timer* tmp = head;
		while (tmp)
		{
			head = tmp->next;
			delete tmp;
			tmp = head;
		}
	}

	void add_timer(util_timer* timer)
	{
		if (!timer)
		{
			return;
		}
		if (!head)
		{
			head = tail = timer;
		}
		// 到期时间早于其他所有的到期时间则插入到头部
		if (timer->expire < head->expire)
		{
			timer->next = head;
			head->prev = timer;
			head = timer;
			return;
		}
		// 到期时间需要插入到链表之中
		add_timer(timer, head);
	}

	// 重置定时器时 调整定时器的位置
	void adjust_timer(util_timer* timer)
	{
		if (!timer)
		{
			return;
		}
		util_timer* tmp = timer->next;
		if (!tmp || (timer->expire < tmp->expire))
		{
			return;
		}

		// 目标定时器是头节点， 将该定时器从链表取出并重新插入
		if (timer == head)
		{
			head = head->next;
			head->prev = nullptr;
			timer->next = nullptr;
			add_timer(timer, head);
		}
		// 如果目标定时器不是头节点， 则插入其所在位置之后的节点
		else
		{
			timer->prev->next = timer->next;
			timer->next->prev = timer->prev;
			add_timer(timer, timer->next); // 第二个参数传入的所在位置之后的节点， 减少时间消耗
		}
	}

	void del_timer(util_timer* timer)
	{
		if (!timer)
		{
			return;
		}
		// 当前只存在一个定时器， 且为目标定时器
		if ((timer == head) && (timer == tail))
		{
			delete timer;
			head = nullptr;
			tail = nullptr;
			return;
		}
		// 当前定时器不只有一个节点， 且目标定时器为头节点， 则调整头节点后删除原头节点
		if (timer == head)
		{
			head = head->next;
			head->prev = NULL;
			delete timer;
			return;
		}
		// 至少有两个计时器， 则将链表的尾节点指向前一个节点
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
		if (!head)
		{
			return;
		}
		printf("timer tick\n");
		time_t cur = time(nullptr);
		util_timer* tmp = head;
		while (tmp)
		{
			if (cur < tmp->expire)
			{
				break;
			}
			// 调用回调函数， 执行定时任务
			tmp->cb_func(tmp->user_data);
			head = tmp->next;
			if (head)
			{
				head->prev = nullptr;
			}
			delete tmp;
			tmp = head;
		}
	}
private:
	void add_timer(util_timer* timer, util_timer* lst_head)
	{
		util_timer* prev = lst_head;
		util_timer* tmp = prev->next;
		while (tmp)
		{
			if (timer->expire < tmp->expire)
			{
				prev->next = timer;
				timer->next = tmp;
				tmp->prev = timer;
				timer->prev = prev;
				break;
			}
			prev = tmp;
			tmp = tmp->next;
		}
		if (!tmp)
		{
			prev->next = timer;
			timer->prev = prev;
			timer->next = nullptr;
			tail = timer;
		}
	}

private:
	util_timer* head;
	util_timer* tail;
};

#endif
