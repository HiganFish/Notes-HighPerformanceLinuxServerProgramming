//
// Created by lsmg on 3/24/20.
//

#include "threadpool.h"
#include "httpconnection.h"

template class ThreadPool<HttpConnection>;

template<typename T>
ThreadPool<T>::ThreadPool(int threadnum, int maxtasks):
thread_number_(threadnum), max_tasks_(maxtasks), stop_(false)
{
    if (thread_number_ <= 0 || max_tasks_ <= 0)
    {
        throw std::exception();
    }
    threads_ = new pthread_t[thread_number_];
    if (!threads_)
    {
        throw std::exception();
    }

    for (int i = 0; i < thread_number_; ++i)
    {
        if (pthread_create(&threads_[i], nullptr, Work, static_cast<void*>(this)) != 0)
        {
            delete [] threads_;
            throw std::exception();
        }
        if (pthread_detach(threads_[i]) != 0)
        {
            delete [] threads_;
            throw std::exception();
        }
    }
}

template<typename T>
ThreadPool<T>::~ThreadPool()
{
    delete []threads_;
    stop_ = true;
}

template<typename T>
void *ThreadPool<T>::Work(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool*>(arg);
    pool->Run();
    return pool;
}

template<typename T>
void ThreadPool<T>::Run()
{
    while (!stop_)
    {
        queuestat_.Wait();

        queue_mutex_.Lock();
        if (work_queue_.empty())
        {
            queue_mutex_.Unlock();
            continue;
        }
        T *request = work_queue_.front();
        work_queue_.pop();
        queue_mutex_.Unlock();

        if (!request)
        {
            continue;
        }
        request->Process();
    }
}
