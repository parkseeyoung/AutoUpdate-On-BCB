//---------------------------------------------------------------------------

#ifndef threadpoolH
#define threadpoolH

#ifdef __cplusplus
/* ���� C++ ��������ָ����C���﷨����*/
extern "C"{
#endif
#define MAX_THREADS 8
#define MAX_QUEUE 65536

/* �򻯱������� */
typedef struct threadpool_t threadpool_t;

/* ��������� */
typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;

typedef enum {
    threadpool_graceful       = 1
} threadpool_destroy_flags_t;

/* �������̳߳��������� API */
/**
 * �����̳߳أ��� thread_count ���̣߳����� queue_size ����������У�flags ����û��ʹ��
 */
threadpool_t *threadpool_create(int thread_count, int queue_size, int flags);

/**
 *  ��������̳߳�, pool Ϊ�̳߳�ָ�룬routine Ϊ����ָ�룬 arg Ϊ���������� flags δʹ��
 */
int threadpool_add(threadpool_t *pool, void (*routine)(void *),
                   void *arg, int flags);


/**
 * �����̳߳أ�flags ��������ָ���رյķ�ʽ
 */
int threadpool_destroy(threadpool_t *pool, int flags);


#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------
#endif
