#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "video_setting.h"

/* system calls */
int loadVideo(const uint8_t*, int, int);
int fullVideo(const unsigned char*);

/* lib functions */
void draw_rectangular(int x, int y, int p, int q, union Pixels c);
void remove_rectangular(int x, int y, int p, int q, const uint8_t* buf);


#endif
