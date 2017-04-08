#include "video.h"

void draw_rectangular(int x, int y, int p, int q, union Pixels c)
{
	uint8_t buffer[SCR_DEPTH] = {c.blue, c.green, c.red};
	int i, j;
	for (i = y; i < q; i++)
		for (j = x; j < p; j++)
			loadVideo(buffer, (i * SCR_WIDTH + j) * SCR_DEPTH, SCR_DEPTH);
}

void remove_rectangular(int x, int y, int p, int q, const uint8_t* buf)
{
	int size = (p - x) * SCR_DEPTH;
	int i;
	for (i = y; i < q; i++)
	{
		int position = (i * SCR_WIDTH + x) * SCR_DEPTH;
		loadVideo(buf + position, position, size);
	}
}
