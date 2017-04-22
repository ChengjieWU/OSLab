/* include libs */
#include "types.h"

#include "printf.h"

#include "proc.h"

void test_main()
{
	printf("Ping!\n");
	int t = fork();
	if (t == 0) printf("Pong!\n");
	else printf("Something is wrong!\n");
	while (1);
}
