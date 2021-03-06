/* include libs */
#include "types.h"
#include "math.h"
#include "string.h"
#include "printf.h"

#include "time.h"
#include "keyboard.h"
#include "video.h"

#include "proc.h"
#include "filesystem.h"

/* Now, game data is regarded as files to read. No need to pre-included. */
/* include data */
//#include "picture.h"

/*--------------------------------------------------------------------------*/

/* global settings */
#define frequency(n) (1000/n)

#define groudWidth SCR_WIDTH
#define groudHeight SCR_HEIGHT
#define groudDepth SCR_DEPTH
#define pixels (groudWidth * groudHeight * groudDepth)

#define ballRadius 8
#define ballTop 300
#define ballLeft 300
#define boardHalfWidth 60
#define boardHalfHeight 8
#define brickRow 5
#define brickColumn 10
#define brickHalfWidth 30
#define brickHalfHeight 12
#define brickHalfSpace 8
#define brickLeft 400
#define brickTop 160
#define boardTop 540

#define ballspeedx 8
#define ballspeedy 8
#define minimumSpeed 6
#define maximumSpeed 12
#define speedSwitch 30
#define flashSwitch frequency(24)
#define step 30


/* To store pictures. */
unsigned char Image_Universe[pixels];
unsigned char Image_Success[pixels];
unsigned char Image_Failure[pixels];

//#define __DEBUG__

/*--------------------------------------------------------------------------*/

/* variables */

uint32_t game_time = 0;
//uint32_t video_refresh_time = 0;
uint32_t current_game = 0;				//This is the raw time read.
//uint32_t current_video = 0;

bool brick[brickRow][brickColumn];
int ball_x, ball_y;
int board_x;
const int board_y = boardTop;
bool is_start;
int v_x, v_y;
int state;
int roundNum = 0;

enum {NOTDECIDED, WIN, LOSE};
enum {NORMAL, QUIT, PAUSE};

/*--------------------------------------------------------------------------*/

/* read game data (images) */
void read_data();

/* video output functions */
void draw_ball();
void remove_ball();
void draw_board();
void remove_board();
void remove_a_brick(int i, int j);
void draw_brick();
void init_display();

/* collision calculation */
void sideORplatform(int x, int y);
bool collide_brick();
bool collide_board();

/* game control */
int round();
void init_Game();

/*--------------------------------------------------------------------------*/

/* We should not use function type int here. If so, the function will try to read value in 0x4(%esp) first. */
/* Since it is loaded by the kernel, %esp = 0xc0000000 at the beginning, and this is the max value. */
/* And when it is first executed, physical frames for user stack hasn't been allocated. */
/* So it will lead to page-protection fault or user-read-page-fault. We should use void. */
/* Compiler won't allow main to be void, so we cannot use main. Instead, we use game_main. */
void game_main(void)
{
	
	printf("We are now in game!\n");

	printf("Loading...\n\n");
	read_data();
	
	printf("*********** Instructions ***********\n");
	printf("README:\nSpace --- pause or start\nQ --- quit\nLeft Arrow --- move left\nRight Arrow --- move right\n\n");
	printf("************************************\n");
	init_Game();

	/* Set the time counter. */
	/*int opid = getpid();
	fork();
	if (getpid() != opid) 
	{
		int i = 0;
		while(true)
		{
			printf("Gametime: %d...\n", i);
			i += 1;;
			sleep(1000);
		}
	}*/


	int key;
	while (true)
	{
		key = readKey();
		if (key == K_Q) break;
		else if (key == K_ENTER || key == K_SPACE) 
		{
			if (state != NOTDECIDED) {init_Game(); continue;}
			int ret = round();
			if (ret == QUIT) break;
			else if (ret == PAUSE) continue;
			if (state == WIN) 
			{
				fullVideo(Image_Success);
				printf("Round %d: You WIN!\n", roundNum++);
			}
			else if (state == LOSE) 
			{
				fullVideo(Image_Failure);
				printf("Round %d: You LOSE!\n", roundNum++);
			}
		}
	}
	memset(Image_Universe, 0xff, sizeof Image_Universe);
	fullVideo(Image_Universe);
	exit();
}

/*--------------------------------------------------------------------------*/

void init_Game()
{
	is_start = false;
	state = NOTDECIDED;
	board_x = 400;
	ball_x = ballLeft;
	ball_y = ballTop;
	memset(brick, true, sizeof brick);
	v_x = ballspeedx;
	v_y = ballspeedy;
	//printf("%d", get_time);
	init_display();
}

void read_data()
{
	int fd = fopen("universe.dat\0", FS_READ);
	if (fd == -1) printf("Read file error!\n");
	fread(fd, Image_Universe, pixels);
	fclose(fd);
	fd = fopen("success.dat\0", FS_READ);
	if (fd == -1) printf("Read file error!\n");
	fread(fd, Image_Success, pixels);
	fclose(fd);
	fd = fopen("failure.dat\0", FS_READ);
	if (fd == -1) printf("Read file error!\n");
	fread(fd, Image_Failure, pixels);
	fclose(fd);
}

int round()
{
	is_start = true;
	while (is_start)
	{	
		int key = readKey();
		switch (key)
		{
			case K_LEFT: if (board_x - step >= boardHalfWidth) {remove_board(); board_x -= step; draw_board();} break; 
			case K_RIGHT: if (board_x + step < groudWidth - boardHalfWidth) {remove_board(); board_x += step; draw_board();} break;
			case K_Q: return QUIT;
			case K_ENTER:
			case K_SPACE: return PAUSE;
			default: break;
		}
			
		current_game = getTime();
		if (game_time != current_game/speedSwitch)
		{
			game_time = current_game/speedSwitch;
			if (ball_x + v_x < ballRadius || ball_x + v_x >= groudWidth - ballRadius) v_x = -v_x;
			if (ball_y + v_y < ballRadius) v_y = -v_y;
			if (ball_y + v_y >= groudHeight - ballRadius)
			{
				state = LOSE;
				is_start = false;
				continue;
			}
			remove_ball();
			ball_x += v_x;
			ball_y += v_y;
			draw_ball();
			draw_board();
			if (collide_board()) v_y = -v_y;
			if (collide_brick())
			{
				state = WIN;
				is_start = false;
			}
		}
	}
	return NORMAL;
}

/*--------------------------------------------------------------------------*/



void draw_ball() 
{
	union Pixels c;
	c.blue = 0; c.green = 0; c.red = 255;
	draw_rectangular(ball_x - ballRadius, ball_y - ballRadius, ball_x + ballRadius, ball_y + ballRadius, c);
}

void remove_ball() 
{
	remove_rectangular(ball_x - ballRadius, ball_y - ballRadius, ball_x + ballRadius, ball_y + ballRadius, Image_Universe);
}

void draw_board()
{
	union Pixels c;
	c.blue = 0; c.green = 255; c.red = 0;
	draw_rectangular(board_x - boardHalfWidth, board_y - boardHalfHeight, board_x + boardHalfWidth, board_y + boardHalfHeight, c);
}

void remove_board()
{
	remove_rectangular(board_x - boardHalfWidth, board_y - boardHalfHeight, board_x + boardHalfWidth, board_y + boardHalfHeight, Image_Universe);
}

void remove_a_brick(int i, int j)
{
	int sx = brickLeft - brickColumn * brickHalfWidth - (brickColumn - 1) * brickHalfSpace;
	int sy = brickTop - brickRow * brickHalfHeight - (brickRow - 1) * brickHalfSpace;
	int x = sx + j * 2 * (brickHalfWidth+brickHalfSpace);
	int y = sy + i * 2 * (brickHalfHeight+brickHalfSpace);
	remove_rectangular(x, y, x + 2 * brickHalfWidth, y + 2 * brickHalfHeight, Image_Universe);
}

void draw_brick()
{
	union Pixels c;
	c.blue = 180; c.green = 50; c.red = 180;
	int i, j;
	int sx = brickLeft - brickColumn * brickHalfWidth - (brickColumn - 1) * brickHalfSpace;
	int sy = brickTop - brickRow * brickHalfHeight - (brickRow - 1) * brickHalfSpace;
	for (i = 0; i < brickRow; i++)
		for (j = 0; j < brickColumn; j++)
			if (brick[i][j])
			{
				int x = sx + j * 2 * (brickHalfWidth+brickHalfSpace);
				int y = sy + i * 2 * (brickHalfHeight+brickHalfSpace);
				draw_rectangular(x, y, x + 2 * brickHalfWidth, y + 2 * brickHalfHeight, c);
			}
}

void init_display()
{
	fullVideo(Image_Universe);
	draw_ball();
	draw_board();
	draw_brick();
}

/*--------------------------------------------------------------------------*/

void sideORplatform(int x, int y)
{
	int o_ball_x = ball_x - v_x;
	int o_ball_y = ball_y - v_y;
	float k = (float)v_x / (float)v_y;
#ifdef __DEBUG__
	printf("%d, %d, %d, %d\n", o_ball_x, x, o_ball_y, y);
#endif
	if (abs(o_ball_x - x) <= brickHalfWidth + ballRadius) {v_y = -v_y; return;}
	else if (abs(o_ball_y - y) <= brickHalfHeight + ballRadius) {v_x = -v_x; return;}
	else
	{
#ifdef __DEBUG__
		printf("haha\n");
#endif
		bool x_axis = o_ball_x > x + boardHalfWidth + ballRadius;
		bool y_axis = o_ball_y > y + boardHalfHeight + ballRadius;
		if (x_axis && y_axis)
		{
			float kd = (o_ball_x - ((float)x + boardHalfWidth + ballRadius))/(o_ball_y - ((float)y + boardHalfHeight + ballRadius));
			if (kd > k) v_y = -v_y;
			else v_x = -v_x;
		}
		else if (x_axis && !y_axis)
		{
			float kd = (o_ball_x - ((float)x + boardHalfWidth + ballRadius))/(o_ball_y - ((float)y - boardHalfHeight - ballRadius));
			if (kd < k) v_y = -v_y;
			else v_x = -v_x;
		}
		else if (!x_axis && y_axis)
		{
			float kd = (o_ball_x - ((float)x - boardHalfWidth - ballRadius))/(o_ball_y - ((float)y + boardHalfHeight + ballRadius));
			if (kd < k) v_y = -v_y;
			else v_x = -v_x;
		}
		else //(!x_axis && !y_axis)
		{
			float kd = (o_ball_x - ((float)x - boardHalfWidth - ballRadius))/(o_ball_y - ((float)y - boardHalfHeight - ballRadius));
			if (kd > k) v_y = -v_y;
			else v_x = -v_x;
		}
	}
}

bool collide_board()
{
	if (abs(board_y-ball_y) <= ballRadius+boardHalfHeight && abs(board_x-ball_x) <= ballRadius+boardHalfWidth) 
	{
		float gap = (float)abs(board_x - ball_x);
		float total = (float)boardHalfWidth;
		if (gap > total/3)
		{
			int safter = v_x - symbolInteger(board_x - ball_x);
			if (gap < 2*total/3 && abs(safter) >= minimumSpeed && abs(safter) <= maximumSpeed) v_x = safter;
			else 
			{	
				safter = v_x - 2 * symbolInteger(board_x - ball_x);
				if (gap >= 2*total/3 && abs(safter) >= minimumSpeed && abs(safter) <= maximumSpeed) v_x = safter;
			}
		}
		//printf("%d\n", v_x);
		return true;
	}
	else return false;
}

bool collide_brick()
{
	int count = 0;
	int i, j;
	int sx = brickLeft - brickColumn * brickHalfWidth - (brickColumn - 1) * brickHalfSpace + brickHalfWidth;
	int sy = brickTop - brickRow * brickHalfHeight - (brickRow - 1) * brickHalfSpace + brickHalfHeight;
	for (i = 0; i < brickRow; i++)
		for (j = 0; j < brickColumn; j++)
			if (brick[i][j])
			{
				int x = sx + j * 2 * (brickHalfWidth + brickHalfSpace);
				int y = sy + i * 2 * (brickHalfHeight + brickHalfSpace);
				if (abs(y - ball_y) <= ballRadius + brickHalfHeight && abs(x - ball_x) <= ballRadius + brickHalfWidth)
				{
					brick[i][j] = false;
					remove_a_brick(i, j);
					count++;
					sideORplatform(x, y);
				}
			}
			else count++;
	if (count == brickColumn * brickRow) return true;
	else return false;
}


/*--------------------------------------------------------------------------*/
