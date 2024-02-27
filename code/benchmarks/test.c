#include <stdio.h>
#include <unistd.h>
#include "../thread-worker.h"


int global_counter, global_counter2;
worker_mutex_t mutex, mutex2;

void dummy_work_short(void *arg)
{
	int i = 0;
	int j = 0;
	int n = *((int *)arg);
	printf("Thread %d running short\n", n);
	for (i = 0; i < 30; i++)
	{
		printf("Thread %d attempting to get mutex\n", n);
		worker_mutex_lock(&mutex);
		printf("Thread %d got mutex\n", n);
		int shared = global_counter;
		for (j = 0; j < 80000; j++)
		{
		}
		global_counter = shared + 1;
		worker_mutex_unlock(&mutex);
		printf("Thread %d unlocked mutex\n", n);
	}
	printf("Thread %d exiting\n", n);
	worker_exit(NULL);
}

void dummy_work_long(void *arg)
{
	int i = 0;
	int j = 0;
	int n = *((int *)arg);
	printf("Thread %d running short\n", n);
	for (i = 0; i < 30; i++)
	{
		printf("Thread %d attempting to get mutex2\n", n);
		worker_mutex_lock(&mutex2);
		printf("Thread %d got mutex2\n", n);
		int shared = global_counter2;
		for (j = 0; j < 30000000; j++)
		{
		}
		global_counter2 = shared + 1;
		worker_mutex_unlock(&mutex2);
		printf("Thread %d unlocked mutex2\n", n);
	}
	printf("Thread %d exiting\n", n);
	worker_exit(NULL);
}


int main(int argc, char **argv)
{
	printf("Running main thread\n");

	worker_mutex_init(&mutex, NULL);
	worker_mutex_init(&mutex2, NULL);

	for (int j = 0; j < 300000000; j++)
	{
	}

	int i = 0;
	int thread_num = 4;
	int *counter = malloc(thread_num * sizeof(int));
	for (i = 0; i < thread_num; i++)
	{
		counter[i] = i + 1;
	}

	worker_t *thread = malloc(thread_num * sizeof(worker_t));

	for (i = 0; i < thread_num; i++) {
		printf("Main thread creating worker thread %d\n", counter[i]);
		// if (i % 2 == 0)
		// {
		// 	worker_create(&thread[i], NULL, &dummy_work_short, &counter[i]);
		// }
		// else
		// {
		// 	worker_create(&thread[i], NULL, &dummy_work_long, &counter[i]);
		// }
		if(i % 2)
			worker_create(&thread[i], NULL, &dummy_work_long, &counter[i]);
		else
			worker_create(&thread[i], NULL, &dummy_work_short, &counter[i]);

	}

	for (i = 0; i < thread_num; i++) {
		printf("Main thread waiting on thread %d\n", counter[i]);
		worker_join(thread[i], NULL);
	}

	printf("Main thread resume\n");
	printf("%d global total increments, expected: 60\n", global_counter);
	printf("%d global2 total increments, expected: 60\n", global_counter2);
	free(thread);
	free(counter);
	worker_mutex_destroy(&mutex);
	worker_mutex_destroy(&mutex2);

	for (int j = 0; j < 30000000; j++)
	{
	}

	printf("Main thread exit\n");
	return 0;
}
