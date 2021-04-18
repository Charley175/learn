/*************************************************************
 *  FileName     : ThreadPool.h
 *  Date         : 2021-4-13
 *  Autor        : Charley
 *  Description  :
 * ***********************************************************/

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <pthread.h>

/**
 *  Task 
*/
typedef void *(*TaskFunc)(void *);

typedef struct {
    TaskFunc Task;     //任务函数
    void    *Arg;      //参数
}ThreadPoolTask_t;

/*线程池管理*/
typedef struct {
   pthread_mutex_t Lock;                  /* 锁住整个结构体 */
   pthread_mutex_t ThreadCount;           /* 用于使用忙线程数时的锁 */
   pthread_cond_t  QueueNotFull;          /* 条件变量，任务队列不为满 */
   pthread_cond_t  QueueNotEmpty;         /* 任务队列不为空 */

   pthread_t *Thread;                     /* 存放线程的tid,实际上就是管理了线 数组 */
   pthread_t Manager;                     /* 管理者线程tid */
   ThreadPoolTask_t *TaskQueue;           /* 任务队列 */

   /*线程池信息*/
   int MinThreadNum;                      /* 线程池中最小线程数 */
   int MaxThreadNum;                      /* 线程池中最大线程数 */
   int LiveThreadNum;                     /* 线程池中存活的线程数 */
   int BusyThreadNum;                     /* 忙线程，正在工作的线程 */
   int WaitDestoryNum;                    /* 需要销毁的线程数 */

   /*任务队列信息*/
   int QueueFront;                        /* 队头 */
   int QueueRear;                         /* 队尾 */
   int QueueSize; 

   /* 存在的任务数 */
   int QueueMaxSize;                      /* 队列能容纳的最大任务数 */
   /*线程池状态*/
   int ShutDown;                          /* true为关闭 */
}ThreadPool_t;

#endif  /*__THREADPOOL_H*/