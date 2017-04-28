#include "types.h"
#include "printf.h"
#include "proc.h"

void test_main()
{
	printf("Ping!\n");
	int id = getpid();
	printf("My pid is %d\n", id);
	int t = fork();
	if (t == 0) printf("Pong!\n");
	else printf("Something is wrong!\n");
	id = getpid();
	printf("My pid is %d\n", id);
	while (1);
}
