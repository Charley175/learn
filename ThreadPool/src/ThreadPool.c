#include "ThreadPool.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Log.h"
#include "errno.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define DEFAULT_TIME 1
#define DEFAULT_THREAD_NUM 5

void ErrorHandle(void ** Pointer)
{
    if ( *Pointer )
    {
        free(*Pointer);
        *Pointer = NULL;    
    }
}

/*工作线程*/
void *WorkThread(void *ThreadPool)
{
    pthread_detach(pthread_self());

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
        }

        /* 判断是否需要清除线程,自杀功能 */
        if ( Pool->WaitDestoryNum > 0 )
        {
            Pool->WaitDestoryNum--;
            /* 判断线程池中的线程数是否大于最小线程数，是则结束当前线程 */
            if (Pool->LiveThreadNum > Pool->MinThreadNum)
            {
                for (int i = 0; i < Pool->MaxThreadNum; i++) 
                {
                    if(pthread_equal(Pool->Thread[i], pthread_self())) {
                        memset(Pool->Thread + i, 0, sizeof(pthread_t));
                        LOG("thread 0x%x is exiting \n", (unsigned int)pthread_self());
                        Pool->LiveThreadNum--;
                        pthread_mutex_unlock(&(Pool->Lock));
                        pthread_exit(NULL); //结束线程
                    }
                }
            }
        }


        /* 线程池开关状态 */
        if (Pool->ShutDown && Pool->QueueSize == 0) //关闭线程池
        {
            pthread_mutex_unlock(&(Pool->Lock));
            LOG("thread 0x%x is exiting \n", (unsigned int)pthread_self());
            pthread_exit(NULL); //线程自己结束自己
        }

        //否则该线程可以拿出任务
        Task.Task = Pool->TaskQueue[Pool->QueueFront].Task; //出队操作
        Task.Arg = Pool->TaskQueue[Pool->QueueFront].Arg;

        Pool->QueueFront = (Pool->QueueFront + 1) % Pool->QueueMaxSize; //环型结构
        Pool->QueueSize--;

        //通知可以添加新任务
        pthread_cond_broadcast(&(Pool->QueueNotFull));

        //释放线程锁
        pthread_mutex_unlock(&(Pool->Lock));

        //执行刚才取出的任务
        LOG("thread 0x%x start working \n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(Pool->ThreadCount)); //锁住忙线程变量
        Pool->BusyThreadNum++;
        pthread_mutex_unlock(&(Pool->ThreadCount));

        (*(Task.Task))(Task.Arg); //执行任务

        //任务结束处理
        WARNING("thread 0x%x end working \n", (unsigned int)pthread_self());
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
    if ( Pool->ShutDown )
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
    Pool->QueueRear = (Pool->QueueRear + 1) % Pool->QueueMaxSize; /* 逻辑环  */
    Pool->QueueSize++;

    /*添加完任务后,队列就不为空了,唤醒线程池中的一个线程*/
    pthread_cond_signal(&(Pool->QueueNotEmpty));
    pthread_mutex_unlock(&(Pool->Lock));

    return 0;
}

/*线程是否存活*/
int is_thread_alive(pthread_t Tid)
{
    int kill_rc = pthread_kill(Tid, 0); //发送0号信号，测试是否存活
    if (kill_rc == ESRCH)               //线程不存在
        return false;

    return true;
}

/*管理线程*/
void *ManagerThread(void *ThreadPool)
{
    pthread_detach(pthread_self());

    int i;
    ThreadPool_t *Pool = (ThreadPool_t *)ThreadPool;

    while ( !Pool->ShutDown )
    {
        sleep(DEFAULT_TIME);                     /*隔一段时间再管理*/
        pthread_mutex_lock(&(Pool->Lock));       /*加锁*/
        int QueueSize = Pool->QueueSize;         /*任务数*/
        int LiveThreadNum = Pool->LiveThreadNum; /*存活的线程数*/
        pthread_mutex_unlock(&(Pool->Lock));     /*解锁*/

        pthread_mutex_lock(&(Pool->ThreadCount));
        int BusyThreadNum = Pool->BusyThreadNum; /*忙线程数*/
        pthread_mutex_unlock(&(Pool->ThreadCount));

        LOG("admin busy live -%d--%d-\n", BusyThreadNum, LiveThreadNum);

        /*创建新线程 实际任务数量大于 最小正在等待的任务数量，存活线程数小于最大线程数*/
        if (QueueSize >= LiveThreadNum && LiveThreadNum <= Pool->MaxThreadNum)
        {
            pthread_mutex_lock(&(Pool->Lock));
            int add = 0;

            /*一次增加 DEFAULT_THREAD_NUM 个线程*/
            for (i = 0; (i < Pool->MaxThreadNum) && (add < DEFAULT_THREAD_NUM) && (Pool->LiveThreadNum < Pool->MaxThreadNum); ++i)
            {
                if (Pool->Thread[i] == 0 || !is_thread_alive(Pool->Thread[i]))
                {
                    if (pthread_create(&(Pool->Thread[i]), NULL, WorkThread, (void *)Pool) == 0)
                    {
                        add++;
                        Pool->LiveThreadNum++;
                    }
                }
            }

            pthread_mutex_unlock(&(Pool->Lock));
        }

        /*销毁多余的线程 忙线程x2 都小于 存活线程，并且存活的大于最小线程数*/
        if ((BusyThreadNum * 2) < LiveThreadNum && LiveThreadNum > Pool->MinThreadNum)
        {
            LOG("admin busy --%d--%d----", BusyThreadNum, LiveThreadNum);
            /*一次销毁DEFAULT_THREAD_NUM个线程*/
            pthread_mutex_lock(&(Pool->Lock));
            Pool->WaitDestoryNum = DEFAULT_THREAD_NUM;
            pthread_mutex_unlock(&(Pool->Lock));

            for (i = 0; i < DEFAULT_THREAD_NUM; ++i)
            {
                //通知正在处于空闲的线程，自杀
                pthread_cond_signal(&(Pool->QueueNotEmpty));
            }
        }
    }
    
    LOG("manager exit \n");
    pthread_exit(NULL);
    return NULL;
}

/**
 * MinThreadNum ：池中最小线程数
 * MaxThreadNum ：池中最大线程数
 * QueueSizeMax ：任务队列大小
*/
ThreadPool_t *PoolInit(int MinThreadNum, int MaxThreadNum, int QueueSizeMax)
{
    if ((MaxThreadNum < MinThreadNum) || MinThreadNum < 1 || QueueSizeMax < 1)
    {
        ERROR("Invalid parameter");
        return NULL;
    }
    ThreadPool_t *Pool = NULL; /*线程池指针*/
    do
    {
        if (!(Pool = (ThreadPool_t *)malloc(sizeof(ThreadPool_t)))) /*为线程池开辟空间*/
        {
            ERROR("malloc error");
            return NULL;
        }
        memset(Pool, 0, sizeof(ThreadPool_t));
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
        if (!Pool->Thread)
        {
            ERROR("Thread Malloc Error");
            ErrorHandle((void **)&Pool);
            return NULL;
        }

        memset(Pool->Thread, 0, sizeof(pthread_t) * MaxThreadNum);

        /*为工作队列开辟空间*/
        Pool->TaskQueue = (ThreadPoolTask_t *)malloc(sizeof(ThreadPoolTask_t) * QueueSizeMax);
        if (!Pool->TaskQueue)
        {
            ERROR("ThskQueue Malloc Error");
            ErrorHandle((void **)&Pool->Thread);
            ErrorHandle((void **)&Pool);
            return NULL;
        }

        memset(Pool->TaskQueue, 0, sizeof(ThreadPoolTask_t) * QueueSizeMax);
        /*初始化锁和条件变量*/
        if (pthread_mutex_init(&(Pool->Lock), NULL) != 0 ||
            pthread_mutex_init(&(Pool->ThreadCount), NULL) != 0 ||
            pthread_cond_init(&(Pool->QueueNotEmpty), NULL) != 0 ||
            pthread_cond_init(&(Pool->QueueNotFull), NULL) != 0)
        {
            ERROR("Lock Init Error");
            ErrorHandle((void **)&Pool->TaskQueue);
            ErrorHandle((void **)&Pool->Thread);
            ErrorHandle((void **)&Pool);
            return NULL;
        }

        /* 启动min_thr_num个工作线程 */
        for (int i = 0; i < Pool->MinThreadNum; ++i)
        {
            /* pool指向当前线程池  threadpool_thread函数在后面讲解 */
            pthread_create(&(Pool->Thread[i]), NULL, WorkThread, (void *)Pool);
            LOG("start thread 0x%x... ", (unsigned int)Pool->Thread[i]);
        }

        /*创建管理线程*/
        pthread_create(&(Pool->Manager), NULL, ManagerThread, (void *)Pool);

        return Pool;
    } while (0);

    return NULL;
}

/*释放线程池*/
int ThreadPoolFree(ThreadPool_t *Pool)
{
    if (Pool == NULL)
        return -1;

    if (Pool->TaskQueue){
        free(Pool->TaskQueue);
        Pool->TaskQueue = NULL;
    }

    if (Pool->Thread)
    {
        free(Pool->Thread);
        Pool->Thread = NULL;
        pthread_mutex_lock(&(Pool->Lock)); /*先锁住再销毁*/
        pthread_mutex_destroy(&(Pool->Lock));
        pthread_mutex_lock(&(Pool->ThreadCount));
        pthread_mutex_destroy(&(Pool->ThreadCount));
        pthread_cond_destroy(&(Pool->QueueNotEmpty));
        pthread_cond_destroy(&(Pool->QueueNotFull));
    }

    free(Pool);
    Pool = NULL;

    return 0;
}

int ThreadPoolDestroy(ThreadPool_t *Pool)
{
    if (!Pool)
        return -1;

    Pool->ShutDown = true;

    /*销毁管理者线程*/
    //  pthread_join(Pool->Manager, NULL);
    
    int i;

    //通知所有线程去自杀(在自己领任务的过程中)
    for (i = 0; i < Pool->LiveThreadNum; ++i)
        pthread_cond_broadcast(&(Pool->QueueNotEmpty));

    /*等待线程结束 先是pthread_exit 然后等待其结束*/
    //  for ( i = 0; i < Pool->MaxThreadNum; ++i )
    //  {
    //     if ( !Pool->Thread[i] )
    //      pthread_join(Pool->Thread[i], NULL);
    //  }
    if (0 != ThreadPoolFree(Pool))
        ERROR("-------------Thread Destory Fail Please Check!-------------");

    return 0;
}
