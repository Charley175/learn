#include "ThreadPool.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Log.h"

static ThreadPool_t *Manager = NULL;
/*工作线程*/
void *WorkThread(void *ThreadPool)
{
  ThreadPool_t *Pool = (ThreadPool_t *)ThreadPool;
  ThreadPoolTask_t Task;

  while (true)
  {
    pthread_mutex_lock(&(Pool->Lock));

    /* 无任务则阻塞在 “任务队列不为空” 上，有任务则跳出 */
    while ((Pool->QueueSize == 0) && (!Pool->ShutDown))
    { 
       LOG("thread 0x%x is waiting \n", (unsigned int)pthread_self());
       pthread_cond_wait(&(Pool->QueueNotEmpty), &(Pool->Lock));

       /* 判断是否需要清除线程,自杀功能 */
       if (Pool->WaitDestoryNum > 0)
       {
          Pool->WaitDestoryNum--;
          /* 判断线程池中的线程数是否大于最小线程数，是则结束当前线程 */
          if (Pool->LiveThreadNum > Pool->MinThreadNum)
          {
             LOG("thread 0x%x is exiting \n", (unsigned int)pthread_self());
             Pool->LiveThreadNum--;
             pthread_mutex_unlock(&(Pool->Lock));
             pthread_exit(NULL);//结束线程
          }
       }
    }

    /* 线程池开关状态 */
    if (Pool->ShutDown) //关闭线程池
    {
       pthread_mutex_unlock(&(Pool->Lock));
       LOG("thread 0x%x is exiting \n", (unsigned int)pthread_self());
       pthread_exit(NULL); //线程自己结束自己
    }

    //否则该线程可以拿出任务
    Task.Task = Pool->TaskQueue[Pool->QueueFront].Task; //出队操作
    Task.Arg = Pool->TaskQueue[Pool->QueueFront].Arg;

    Pool->QueueFront = (Pool->QueueFront + 1) % Pool->QueueMaxSize;  //环型结构
    Pool->QueueSize--;

    //通知可以添加新任务
    pthread_cond_broadcast(&(Pool->QueueNotFull));

    //释放线程锁
    pthread_mutex_unlock(&(Pool->Lock));

    //执行刚才取出的任务
    printf("thread 0x%x start working \n", (unsigned int)pthread_self());
    pthread_mutex_lock(&(Pool->ThreadCount));            //锁住忙线程变量
    Pool->BusyThreadNum++;
    pthread_mutex_unlock(&(Pool->ThreadCount));

    (*(Task.Task))(Task.Arg);                           //执行任务

    //任务结束处理
    printf("thread 0x%x end working \n", (unsigned int)pthread_self());
    pthread_mutex_lock(&(Pool->ThreadCount));
    Pool->BusyThreadNum--;
    pthread_mutex_unlock(&(Pool->ThreadCount));
  }

  pthread_exit(NULL);
}

/*向线程池的任务队列中添加一个任务*/
int AddTask(ThreadPool_t *Pool, TaskFunc Func, void *Arg)
{
   pthread_mutex_lock(&(Pool->Lock));

   /*如果队列满了,调用wait阻塞*/
   while ((Pool->QueueSize == Pool->QueueMaxSize) && (!Pool->ShutDown))
      pthread_cond_wait(&(Pool->QueueNotFull), &(Pool->Lock));

   /*如果线程池处于关闭状态*/
   if (Pool->ShutDown)
   {
      pthread_mutex_unlock(&(Pool->Lock));
      return -1;
   }

//    /*清空工作线程的回调函数的参数arg*/
//    if (Pool->TaskQueue[Pool->QueueRear].Arg != NULL)
//    {
//       free(Pool->TaskQueue[Pool->QueueRear].Arg);
//       Pool->TaskQueue[Pool->QueueRear].Arg = NULL;
//    }

   /*添加任务到任务队列*/
   Pool->TaskQueue[Pool->QueueRear].Task = Func;
   Pool->TaskQueue[Pool->QueueRear].Arg = Arg;
   Pool->QueueRear = (Pool->QueueRear + 1) % Pool->QueueMaxSize;  /* 逻辑环  */
   Pool->QueueSize++;

   /*添加完任务后,队列就不为空了,唤醒线程池中的一个线程*/
   pthread_cond_signal(&(Pool->QueueNotEmpty));
   pthread_mutex_unlock(&(Pool->Lock));

   return 0;
}

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
        /* 启动min_thr_num个工作线程 */
        for (int i =0; i < Pool->MinThreadNum; ++i )
        {
            /* pool指向当前线程池  threadpool_thread函数在后面讲解 */
            pthread_create(&(Pool->Thread[i]), NULL, WorkThread, (void *)Pool);
            LOG("start thread 0x%x... ", (unsigned int)Pool->Thread[i]);
        }

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