/*********************************************************************************
 *      Copyright:  (C) 2017 zoulei
 *                  All rights reserved.
 *
 *       Filename:  snake.c
 *    Description:  This file
 *
 *        Version:  1.0.0(2017年09月09日)
 *         Author:  zoulei <zoulei121@gmail.com>
 *      ChangeLog:  1, Release initial version on "2017年09月09日 17时05分19秒"
 *
 ********************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define NUM 60
 
struct direct                //用来表示方向的
{
    int cx;
    int cy;
};
typedef struct node            //链表的结点
{
    int cx;
    int cy;
    struct node *back;
    struct node *next;
}node;
 
void initGame();            //初始化游戏
int setTicker(int);            //设置计时器
void show();                //显示整个画面
void showInformation();        //显示游戏信息（前两行）
void showSnake();            //显示蛇的身体
void getOrder();            //从键盘中获取命令
void over(int i);            //完成游戏结束后的提示信息
 
void creatLink();                //（带头尾结点）双向链表以及它的操作
void insertNode(int x, int y);
void deleteNode();
void deleteLink();
 
int ch;                                //输入的命令
int hour, minute, second;            //时分秒
int length, tTime, level;            //（蛇的）长度，计时器，（游戏）等级
struct direct dir, food;            //蛇的前进方向，食物的位置
node *head, *tail;                    //链表的头尾结点

void connectToServer(char* ip, char* port);
void sendMsgToServer(char* msg);
int sock;

int main(int argc, char* argv[])
{
	
	if(argc <= 2) {
		printf("Wrong number of parameters\n");
		return 1;
	}

	connectToServer(argv[1], argv[2]);

    initscr();
    initGame();
    signal(SIGALRM, show);
    getOrder();
    endwin();
    return 0;
}
 
void initGame()
{
    cbreak();                    //把终端的CBREAK模式打开
    noecho();                    //关闭回显
    curs_set(0);                //把光标置为不可见
    keypad(stdscr, true);        //使用用户终端的键盘上的小键盘
    srand(time(0));                //设置随机数种子
    //初始化各项数据
    hour = minute = second = tTime = 0;
    length = 1;
    dir.cx = 1;
    dir.cy = 0;
    ch = 'A';
    food.cx = rand() % COLS;
    food.cy = rand() % (LINES-2) + 2;
    creatLink();
    setTicker(20);
}
 
//设置计时器（这个函数是书本上的例子，有改动）
int setTicker(int n_msecs)
{
    struct itimerval new_timeset;
    long    n_sec, n_usecs;
 
    n_sec = n_msecs / 1000 ;
    n_usecs = ( n_msecs % 1000 ) * 1000L ;
    new_timeset.it_interval.tv_sec  = n_sec;
    new_timeset.it_interval.tv_usec = n_usecs;
    n_msecs = 1;
    n_sec = n_msecs / 1000 ;
    n_usecs = ( n_msecs % 1000 ) * 1000L ;
    new_timeset.it_value.tv_sec     = n_sec  ;
    new_timeset.it_value.tv_usec    = n_usecs ;
    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}
 
void showInformation()
{
    tTime++;
    if(tTime >= 1000000)                //
        tTime = 0;
    if(1 != tTime % 50)
        return;
    move(0, 3);
    //显示时间
    printw("time: %d:%d:%d %c", hour, minute, second);
    second++;
    if(second > NUM)
    {
        second = 0;
        minute++;
    }
    if(minute > NUM)
    {
        minute = 0;
        hour++;
    }
    //显示长度，等级
    move(1, 0);
    int i;
    for(i=0;i<COLS;i++)
        addstr("-");
    move(0, COLS/2-5);
    printw("length: %d", length);
    move(0, COLS-10);
    level = length / 3 + 1;
    printw("level: %d", level);
}
 
//蛇的表示是用一个带头尾结点的双向链表来表示的，
//蛇的每一次前进，都是在链表的头部增加一个节点，在尾部删除一个节点
//如果蛇吃了一个食物，那就不用删除节点了
void showSnake()
{
    if(1 != tTime % (30-level))
        return;
    //判断蛇的长度有没有改变
    bool lenChange = false;
    //显示食物
    move(food.cy, food.cx);
    printw("@");
    //如果蛇碰到墙，则游戏结束
    if((COLS-1==head->next->cx && 1==dir.cx)
        || (0==head->next->cx && -1==dir.cx)
        || (LINES-1==head->next->cy && 1==dir.cy)
        || (2==head->next->cy && -1==dir.cy))
    {
        over(1);
        return;
    }
    //如果蛇头砬到自己的身体，则游戏结束
    if('*' == mvinch(head->next->cy+dir.cy, head->next->cx+dir.cx) )
    {
        over(2);
        return;
    }
    insertNode(head->next->cx+dir.cx, head->next->cy+dir.cy);
    //蛇吃了一个“食物”
    if(head->next->cx==food.cx && head->next->cy==food.cy)
    {
        lenChange = true;
        length++;
        //恭喜你，通关了
        if(length >= 50)
        {
            over(3);
            return;
        }
        //重新设置食物的位置
        food.cx = rand() % COLS;
        food.cy = rand() % (LINES-2) + 2;
    }
    if(!lenChange)
    {
        move(tail->back->cy, tail->back->cx);
        printw(" ");
        deleteNode();
    }
    move(head->next->cy, head->next->cx);
    printw("*");
}
 
void show()
{
    signal(SIGALRM, show);        //设置中断信号
    showInformation();
    showSnake();
    refresh();                    //刷新真实屏幕
}
 
void getOrder()
{
    //建立一个死循环，来读取来自键盘的命令
    while(1)
    {
        ch = getch();
        if(KEY_LEFT == ch)
        {
            dir.cx = -1;
            dir.cy = 0;
        }
        else if(KEY_UP == ch)
        {
            dir.cx = 0;
            dir.cy = -1;
        }
        else if(KEY_RIGHT == ch)
        {
            dir.cx = 1;
            dir.cy = 0;
        }
        else if(KEY_DOWN == ch)
        {
            dir.cx = 0;
            dir.cy = 1;
        }
        setTicker(20);
    }
}
 
void over(int i)
{
    //显示结束原因
    move(0, 0);
    int j;
    for(j=0;j<COLS;j++)
        addstr(" ");
    move(0, 2);
    if(1 == i)
        addstr("Crash the wall. Game over");
    else if(2 == i)
        addstr("Crash itself. Game over");
    else if(3 == i)
        addstr("Mission Complete");
    setTicker(0);                //关闭计时器
    deleteLink();                //释放链表的空间
}
void sendMsgToServer(char* msg) {
	send(sock, msg, strlen(msg), 0);
}

void connectToServer(char* ip, char* port) {
	printf("%s-%s\n", ip, port);
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(port));
	inet_pton(AF_INET, ip, &address.sin_addr);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock >= 0);

	int conn = connect(sock, (struct sockaddr*)&address, sizeof(address));
	printf("error is %s\n", strerror(errno));
	assert(conn >= 0);
}
 
//创建一个双向链表
void creatLink()
{
    node *temp = (node *)malloc( sizeof(node) );
    head = (node *)malloc( sizeof(node) );
    tail = (node *)malloc( sizeof(node) );
    temp->cx = 5;
    temp->cy = 10;
    head->back = tail->next = NULL;
    head->next = temp;
    temp->next = tail;
    tail->back = temp;
    temp->back = head;
}
 
//在链表的头部（非头结点）插入一个结点
void insertNode(int x, int y)
{
    char msg[1024];
    memset(msg, '\0', 1024);
    sprintf(msg, "The X is %d, The Y is %d\n", x, y);

    sendMsgToServer(msg);

    node *temp = (node *)malloc( sizeof(node) );
    temp->cx = x;
    temp->cy = y;
    temp->next = head->next;
    head->next = temp;
    temp->back = head;
    temp->next->back = temp;
}
 
//删除链表的（非尾结点的）最后一个结点
void deleteNode()
{
    node *temp = tail->back;
    node *bTemp = temp->back;
    bTemp->next = tail;
    tail->back = bTemp;
    temp->next = temp->back = NULL;
    free(temp);
    temp = NULL;
}
 
//删除整个链表
void deleteLink()
{
    while(head->next != tail)
        deleteNode();
    head->next = tail->back = NULL;
    free(head);
    free(tail);
}
