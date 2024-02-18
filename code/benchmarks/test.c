#include <stdio.h>
#include <unistd.h>
#include "../thread-worker.h"
#include "../dlist.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

int main(int argc, char **argv)
{
	dlist test;
	init_list(&test);
	tcb t1,t2,t3;
	t1.id = 1;
	t1.priority = 5;

	t2.id = 2;
	t2.priority = 6;

	t3.id = 3;
	t3.priority = -1;

	list_front_insert(&test,&t1);
	list_back_insert(&test,&t3);
	list_insert(&test,&t2,1);
	/* Implement HERE */

	printf("%d %d %d\n", list_get(&test,1)->priority,list_get(&test,2)->priority,list_get(&test,3)->priority);
	printf("%d\n", test.head->data->id);
	printf("%d\n", test.back->data->id);
	return 0;
}
