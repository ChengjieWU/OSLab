#include "types.h"
#include "printf.h"
#include "proc.h"

void test_main()
{
	fork();
	int t = getpid();
	if (t == 0)
	{
		printf("Ping\n");
		exit();
	}
	else 
	{
		printf("Pong\n");
		while (true);
	}
}
