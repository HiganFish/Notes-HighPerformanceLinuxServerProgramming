#include <sys/msg.h>
#include <cstdio>
#include <cstring>

struct msg_buf
{
    long mtype;
    char mtext[256];
};

int main()
{
    int msqid = msgget(123, 0666 |IPC_CREAT);
    if (msqid != -1)
    {
        msg_buf msg_b{};
	msg_b.mtype = 1;
	strcpy(msg_b.mtext, "I'm send process\n");

	if (msgsnd(msqid, &msg_b, sizeof(msg_b.mtext), 0) == 0)
    	{
	    printf("send success\n");
	}
	else
	{
	    printf("send failure\n");
	}
    }

    return 0;
}
