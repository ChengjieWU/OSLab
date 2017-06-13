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
	
	/* In fact, for size reason, I modified boot, making it only works for having exboot. */
	/* So exboot > 0 is a must. */
	if (EXBOOT <= 0) printf("ERROR!\n");
	if (EXBOOT > 0) 
	{
		FILE *exboot = fopen("exboot.bin", "rb");
		fseek(exboot, 0, SEEK_END);
		unsigned exboot_size = ftell(exboot);
		fseek(exboot, 0, SEEK_SET);
		int exboot_block = exboot_size / blocksize + !!(exboot_size % blocksize);
		
		if (exboot_block > EXBOOT) printf("exboot too large! ERROR!\n");
		else printf("OK: extended boot is %d bytes (max %d)\n", exboot_size, EXBOOT * blocksize);
		
		int extended_boot;
		for (extended_boot = 0; extended_boot < exboot_block; extended_boot++)
		{	
			memset(buffer, 0, sizeof buffer);
			fread(buffer, blocksize, 1, exboot);
			fwrite(buffer, blocksize, 1, disk);
		}
		/* Generate empty parts. */
		memset(buffer, 0, sizeof buffer);
		for (extended_boot = exboot_block; extended_boot < EXBOOT; extended_boot++)
			fwrite(buffer, blocksize, 1, disk);
		fclose(exboot);
		
	}
	
	memset(buffer, 0, sizeof buffer);
	/* Bitmap, which is occupied at first. */
	if (DATAOFFSET / 8 + !!(DATAOFFSET % 8) > blocksize) printf("ERROR!\n");
	
	int i;
	for (i = 0; i < DATAOFFSET / 8 + !!(DATAOFFSET % 8); i++) {
		buffer[i] = 0xff;
	}
	fwrite(buffer, blocksize, 1, disk);
	
	memset(buffer, 0, sizeof buffer);
	for (i = 0; i < DISKBLOCK - 1 - BMAPOFFSET; i++)
		fwrite(buffer, blocksize, 1, disk);
	fclose(disk);
}
