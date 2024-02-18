#include <stdio.h>
#include <unistd.h>

#define WORKER_DEBUG
#include "../thread-worker.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

int main(int argc, char **argv)
{
	tcb* run_queue = NULL;
	tcb t1,t2,t3;
	t1.id = 1;
	t1.priority = 5;

	t2.id = 2;
	t2.priority = 6;

	t3.id = 3;
	t3.priority = -1;

	emplace_back(&run_queue,&t1);
	printf("%d\n", back(run_queue)->priority);

	emplace_back(&run_queue,&t2);
	printf("%d\n", back(run_queue)->priority);

	emplace_back(&run_queue,&t3);
	printf("%d\n", back(run_queue)->priority);

	remove_elem(&run_queue,&t2);
	printf("%d\n", back(run_queue)->priority);

	pop_front(&run_queue);
	printf("%d\n", run_queue->priority);
	return 0;
}
