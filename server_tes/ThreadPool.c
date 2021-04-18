#include "ThreadPool.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Log.h"

static ThreadPool_t *Manager = NULL;


/**
 * MinThreadNum ：池中最小线程数
 * MaxThreadNum ：池中最大线程数
 * QueueSizeMax ：任务队列大小
*/
ThreadPool_t *PoolInit(int MinThreadNum, int MaxThreadNum, int QueueSizeMax)
{
    if ( (MaxThreadNum < MinThreadNum) || MinThreadNum < 1 || QueueSizeMax < 1 )
    {
        ERROR("Invalid parameter");
        return NULL;
    }

    /*线程池指针*/
    ThreadPool_t *Pool = NULL;

    do {

        /*为线程池开辟空间*/
        if ( !(Pool = (ThreadPool_t *)malloc(sizeof(ThreadPool_t))) )
        {
            ERROR("malloc error");
            return NULL;
        }

        /*信息初始化*/
        Pool->MinThreadNum = MinThreadNum;
        Pool->MaxThreadNum = MaxThreadNum;
        Pool->BusyThreadNum = 0;
        Pool->LiveThreadNum = MinThreadNum;
        Pool->WaitDestoryNum = 0;
        Pool->QueueFront = 0;
        Pool->QueueRear = 0;
        Pool->QueueSize = 0;
        Pool->QueueMaxSize = QueueSizeMax;
        Pool->ShutDown = false;
        /*为线程池开辟最多线程数的空间*/
        Pool->Thread = (pthread_t *)malloc(sizeof(pthread_t) * MaxThreadNum);
        if ( !Pool->Thread )
        {
            ERROR("Thread Malloc Error");
            return NULL;
        }

        /*为工作队列开辟空间*/
        Pool->TaskQueue = (ThreadPoolTask_t *)malloc(sizeof(ThreadPoolTask_t) * QueueSizeMax);
        if (!Pool->TaskQueue)
        {
            ERROR("ThskQueue Malloc Error");
            return NULL;
        }

        /*初始化锁和条件变量*/
        if (pthread_mutex_init(&(Pool->Lock), NULL) != 0 ||\
            pthread_mutex_init(&(Pool->ThreadCount), NULL) != 0 ||\
            pthread_cond_init(&(Pool->QueueNotEmpty), NULL)!= 0 ||\
            pthread_cond_init(&(Pool->QueueNotFull), NULL)!= 0\
        ){
            ERROR("Lock Init Error");
            return NULL;
        }

        /*创建工作线程*/
        /**
         *......
        */

        /*创建管理线程*/
       /**
         *......
        */
       return Pool;
    }while (0);

    return NULL;
}

void ThreadPoolDestory(ThreadPool_t *Pool)
{


}

int main(void)
{
    PoolInit(10, 20, 20);
    return 0;
}