#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#pragma pack(0)
struct bitmap{
	unsigned char mask[512 * 3];
} bmap;
#pragma pack()

unsigned char tmp[512];


int main()
{
	srand((unsigned)time(NULL));
	int i;
	for (i = 0; i < 1250; i++)
	{
		bmap.mask[i] = rand() % 256;
	}
	FILE *fp = fopen("bitmap", "wb");
	fwrite(bmap.mask, 1, 512 * 3, fp);
	memset(tmp, 1, 512);
	for (i = 0; i < 4; i++)
	{
		fwrite(tmp, 1, 512, fp);
	}
	memset(tmp, 2, 512);
	fwrite(tmp, 1, 512, fp);
	fclose(fp);
}
