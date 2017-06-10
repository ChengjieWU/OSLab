#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma pack(0)
struct bitmap{
	unsigned char mask[512];
} bmap;
#pragma pack()


int main()
{
	srand((unsigned)time(NULL));
	int i;
	for (i = 0; i < 512; i++)
	{
		bmap.mask[i] = rand() % 256;
	}
	FILE *fp = fopen("bitmap", "wb");
	fwrite(bmap.mask, 1, 512, fp);
	fclose(fp);
}
