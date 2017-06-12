#include <stdio.h>
#include <string.h>
#include "fs.h"

unsigned char buffer[blocksize];
struct dir root;
struct inode Inode[FILENUM];


int main()
{
	FILE *disk = fopen("disk.bin", "rb");
	
	fseek(disk, ROOTOFFSET * blocksize, SEEK_SET);
	fread((unsigned char *)&root, blocksize, 1, disk);
	fseek(disk, INODEOFFSET * blocksize, SEEK_SET);
	fread((unsigned char *)Inode, blocksize * FILENUM, 1, disk);
	
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
	{
		if (root.entries[i].file_size != 0)
		{
			FILE *file = fopen((const char *)root.entries[i].filename, "wb");
			int j = root.entries[i].inode_offset;
			int complete_block = root.entries[i].file_size / blocksize;
			int rest_bytes = root.entries[i].file_size % blocksize;
			
			int count = 0;
			
			int k = 0;
			while (Inode[j].data_block_offsets[k] != 0)
			{
				//printf("%d\n", Inode[j].data_block_offsets[k]);
				fseek(disk, Inode[j].data_block_offsets[k] * blocksize, SEEK_SET);
				if (count == complete_block)
				{
					fread(buffer, rest_bytes, 1, disk);
					fwrite(buffer, rest_bytes, 1, file);
				}
				else
				{
					fread(buffer, blocksize, 1, disk);
					fwrite(buffer, blocksize, 1, file);
				}
				count += 1;
				k++;
				if (k == blocksize / sizeof(unsigned) - 1)
				{
					if (Inode[j].data_block_offsets[k] == 0) break;
					else
					{
						//printf("%d\n", Inode[j].data_block_offsets[k]);
						j = Inode[j].data_block_offsets[k];
						k = 0;
					}
				}
			}
			fclose(file);
		}
	}
	
	fclose(disk);
	return 0;
}
