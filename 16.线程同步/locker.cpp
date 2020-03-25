//
// Created by lsmg on 3/23/20.
//

#ifndef PTHREAD_LOCKER_H
#define PTHREAD_LOCKER_H

#include <semaphore.h>
#include <pthread.h>
#include <exception>

class Sem
{
    Sem()
    {
        if (sem_init(&sem_, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    ~Sem()
    {
        sem_destroy(&sem_);
    }
    bool Wait()
    {
        return sem_wait(&sem_) == 0;
    }
    bool Post()
    {
        return sem_post(&sem_) == 0;
    }
private:
    sem_t sem_;
};

class Mutex
{
    Mutex()
    {
        if (pthread_mutex_init(&mutex_, nullptr) != 0)
        {
            throw std::exception();
        }

    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }
    bool Lock()
    {
        return pthread_mutex_lock(&mutex_) == 0;
    }
    bool Unlock()
    {
        return pthread_mutex_unlock(&mutex_) == 0;
    }

private:
    pthread_mutex_t mutex_;
};

class Cond
{
public:
    Cond()
    {
        if (pthread_mutex_init(&mutex_, nullptr) != 0)
        {
            throw std::exception();
        }
        if (pthread_cond_init(&cond_, nullptr) != 0)
        {
            // 
            pthread_mutex_destroy(&mutex_);
            throw std::exception();
        }
    }
    ~Cond()
    {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cond_);
    };
    bool Wait()
    {
        int ret = 0;
        pthread_mutex_lock(&mutex_);
        ret = pthread_cond_wait(&cond_, &mutex_);
        pthread_mutex_unlock(&mutex_);
        return ret == 0;
    }
    bool Signal()
    {
        return pthread_cond_signal(&cond_) == 0;
    }
private:
    pthread_cond_t cond_;
    pthread_mutex_t mutex_;
};

#endif //PTHREAD_LOCKER_H
