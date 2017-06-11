#include <stdio.h>
#include <string.h>
#include "fs.h"

unsigned char buffer[blocksize];

int main()
{
	FILE *boot = fopen("boot.bin", "rb");
	fread(buffer, blocksize, 1, boot);
	fclose(boot);
	FILE *disk = fopen("disk.bin", "wb");
	fwrite(buffer, blocksize, 1, disk);
	
	memset(buffer, 0, sizeof buffer);
	
	/* Bitmap, which is occupied at first. */
	buffer[0] = 0xff;
	buffer[1] = 0xff;
	fwrite(buffer, blocksize, 1, disk);
	
	memset(buffer, 0, sizeof buffer);
	int i;
	for (i = 0; i < DISKBLOCK - 2; i++)
		fwrite(buffer, blocksize, 1, disk);
	fclose(disk);
}
