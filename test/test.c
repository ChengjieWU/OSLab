#include "types.h"
#include "printf.h"
#include "proc.h"

void test_main()
{
	fork();
	int t = getpid();
	//printf("I'm process No.%d\n", t);
	while(1) 
	{
		if (t == 0) printf("i\n");
		else if (t == 1) printf("oo\n");
	}
	exit();
}
