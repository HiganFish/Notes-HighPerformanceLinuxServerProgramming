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

	if (msgrcv(msqid, &msg_b, sizeof(msg_b.mtext), 0, IPC_NOWAIT) != -1)
    	{
	    printf("read success: %s\n", msg_b.mtext);

	    msgctl(msqid, IPC_RMID, 0);
	}
    }

    return 0;
}
