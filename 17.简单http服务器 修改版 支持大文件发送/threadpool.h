//
// Created by lsmg on 3/24/20.
//

#ifndef PTHREAD_THREADPOOL_H
#define PTHREAD_THREADPOOL_H

#include <queue>
#include "locker.h"

template<typename T>
class ThreadPool
{
public:
    explicit ThreadPool(int threadnum = 8, int maxtasks = 10000);
    ~ThreadPool();

    void Append(T* work) {work_queue_.push(work); queuestat_.Post();}
private:
    static void* Work(void *arg);

    void Run();
private:
    // 线程数量
    int thread_number_;
    // 最大任务数量
    int max_tasks_;
    // 线程池数组， 大小为thread_number
    pthread_t *threads_;
    // 工作队列
    std::queue<T*> work_queue_;
    // 保护工作队列的锁
    Mutex queue_mutex_;
    // 是否有新任务要处理
    Sem queuestat_;
    // 是否结束线程
    bool stop_;
};

#endif //PTHREAD_THREADPOOL_H
