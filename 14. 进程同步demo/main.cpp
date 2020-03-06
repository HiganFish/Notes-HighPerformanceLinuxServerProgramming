#include <sys/sem.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>

union semun
{
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO(Linux-specific) */
};

bool P(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; // P
    sem_b.sem_flg = SEM_UNDO;
    return semop(sem_id, &sem_b, 1) != -1;
}

int V(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; // V
    sem_b.sem_flg = SEM_UNDO;
    return semop(sem_id, &sem_b, 1) != -1;
}

int main(int argc, char *argv[])
{
    char message = 'x';

    int sem_id = semget((key_t) 1234, 1, 0666 | IPC_CREAT);

    // 初次调用程序初始化信号量
    if (argc > 1)
    {
        // 初始化信号量
        union semun sem_union;
        sem_union.val = 1;

        if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
        {
            exit(0);
        }
        message = argv[1][0];
        sleep(2);
    }

    for (int i = 0; i < 10; ++i)
    {
        if (!P(sem_id))
        {
            exit(EXIT_FAILURE);
        }
        // 打印自己的特有message
        printf("%c", message);
        fflush(stdout);

        sleep(rand() % 3);

        printf("%c", message);
        fflush(stdout);

        if (!V(sem_id))
        {
            exit(EXIT_FAILURE);
        }
    }

    sleep(2);
    printf("\n%d - finished\n", getpid());

    // 初次调用完删除信号量
    if (argc > 1)
    {
        sleep(3);
        // 删除信号量
        union semun sem_union{};
        if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}