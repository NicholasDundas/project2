#include <stdio.h>
#include <unistd.h>
#include "../thread-worker.h"

#define DEFAULT_THREAD_NUM 6

int global_counter;
worker_mutex_t* mutex;

void dummy_work_short(void *arg)
{
	int i = 0;
	int j = 0;
	int n = *((int *)arg);

	for (i = 0; i < 700; i++)
	{
		for (j = 0; j < 80000; j++)
		{
		}
		printf("Thread %d running\n", n);
		worker_yield();
	}

	printf("Thread %d exiting\n", n);
	worker_exit(NULL);
}

void dummy_work_long(void *arg)
{
	int i = 0;
	int j = 0;
	int n = *((int *)arg);

	for (i = 0; i < 75; i++)
	{
		for (j = 0; j < 30000000; j++)
		{
		}
		printf("Thread %d running long\n", n);
	}

	printf("Thread %d exiting\n", n);
	worker_exit(NULL);
}

void mutex_work(void *arg)
{
	int i = 0;
	int j = 0;
	int n = *((int *)arg);

	for (i = 0; i < 7; i++)
	{
		worker_mutex_lock(mutex);
		for (j = 0; j < 8000000; j++)
		{
			global_counter++;
		}
		worker_mutex_unlock(mutex);
		printf("Thread %d running\n", n);
	}

	printf("Thread %d exiting\n", n);
	worker_exit(NULL);
}

int main(int argc, char **argv)
{
	int thread_num;
	if (argc == 1)
	{
		thread_num = DEFAULT_THREAD_NUM;
	}
	else
	{
		if (argv[1] < 1)
		{
			printf("enter a valid thread number\n");
			return 0;
		}
		else
		{
			thread_num = atoi(argv[1]);
		}
	}

	printf("Running main thread\n");

	mutex = (worker_mutex_t*) malloc(sizeof(worker_mutex_t));
	worker_mutex_init(mutex, NULL);

	int i = 0;

	int *counter = (int *)malloc(thread_num * sizeof(int));
	for (i = 0; i < thread_num; i++)
	{
		counter[i] = i + 1;
	}

	worker_t *thread = (worker_t *)malloc(thread_num * sizeof(worker_t));

	for (i = 0; i < thread_num; i++)
	{
		printf("Main thread creating worker thread %d\n", counter[i]);
		// if (i % 2 == 0)
		// {
		// 	worker_create(&thread[i], NULL, &dummy_work_short, &counter[i]);
		// }
		// else
		// {
		// 	worker_create(&thread[i], NULL, &dummy_work_long, &counter[i]);
		// }
		worker_create(&thread[i], NULL, &mutex_work, &counter[i]);
	}

	for (i = 0; i < thread_num; i++)
	{
		printf("Main thread waiting on thread %d\n", counter[i]);
		worker_join(thread[i], NULL);
	}

	printf("Main thread resume\n");
	printf("%d total increments\n", global_counter);
	free(thread);
	free(counter);
	worker_mutex_destroy(mutex);

	printf("Main thread exit\n");
	return 0;
}
