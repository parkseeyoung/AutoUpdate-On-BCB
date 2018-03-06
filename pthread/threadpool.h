// threadpool.h
// gmm 2010-3-16 initial code
//
// Multi-threading a Web Service is essential when the response times for
// handling requests by the service are (potentially) long or when keep-alive
// is enabled. 
//
// Using a pool of servers to limit the machine's resource utilization.
//
// 2011-3-25
//     convert MAX_SERVER_THREADS, MAX_REQUEST_QUEUE to template parameters.
//     pthread_create() can fail; so check actual number of threads in code.
//
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

template<class T, int _max_threads = 10, int _max_queue = 16>
class soapThreadPool {
public:
	typedef soapThreadPool<T, _max_threads, _max_queue> thisClass;
	static const int MAX_SERVER_THREADS = _max_threads;
	static const int MAX_REQUEST_QUEUE = _max_queue;

	soapThreadPool()
	{
		num_threads = 0;
		head = tail = 0;
		pthread_mutex_init(&queue_cs, NULL);
		pthread_cond_init(&queue_cv, NULL);
	}

	~soapThreadPool()
	{
		pthread_mutex_destroy(&queue_cs);
		pthread_cond_destroy(&queue_cv);
	}

	int create(T *service)
	{
		for (num_threads = 0; num_threads < MAX_SERVER_THREADS; num_threads++) {
			soapInstance[num_threads] = new T(*service);
			soapInstance[num_threads]->user = this;
			if (pthread_create(&tid[num_threads], NULL, process_queue, soapInstance[num_threads]))
				break;
		}

		return num_threads;
	}

	void destroy()
	{
		int i;

		for (i = 0; i < num_threads; i++) {
			while (enqueue(SOAP_INVALID_SOCKET) == SOAP_EOM)
				Sleep(1);
		}

		for (i = 0; i < num_threads; i++) {
			pthread_join(tid[i], NULL);
			delete soapInstance[i];
			soapInstance[i] = NULL;
		}
	}

	int enqueue(SOAP_SOCKET sock)
	{
		int status = SOAP_OK;
		int next;
		pthread_mutex_lock(&queue_cs);
		next = tail + 1;
		if (next >= MAX_REQUEST_QUEUE)
			next = 0;
		if (next == head)
			status = SOAP_EOM;
		else {
			queue[tail] = sock;
			tail = next;
		}
		pthread_cond_signal(&queue_cv);
		pthread_mutex_unlock(&queue_cs);
		return status;
	}

	SOAP_SOCKET dequeue()
	{
		SOAP_SOCKET sock;
		pthread_mutex_lock(&queue_cs);
		while (head == tail)
			pthread_cond_wait(&queue_cv, &queue_cs);
		sock = queue[head++];
		if (head >= MAX_REQUEST_QUEUE)
			head = 0;
		pthread_mutex_unlock(&queue_cs);
		return sock;
	} 

	static void *process_queue(void *arg)
	{
		T *soapService = (T *)arg;
		thisClass *This = (thisClass *)soapService->user;

		for (;;) {
			soapService->socket = This->dequeue();
			if (!soap_valid_socket(soapService->socket))
				break;
			soapService->serve();
			soapService->destroy();
			soap_free_cookies(soapService);
		}

		return NULL;
	}

private:
	int num_threads;
	T *soapInstance[MAX_SERVER_THREADS];
	pthread_t tid[MAX_SERVER_THREADS];
	SOAP_SOCKET queue[MAX_REQUEST_QUEUE];
	int head, tail; // Queue head and tail
	pthread_mutex_t queue_cs;
	pthread_cond_t queue_cv;
};

#endif
