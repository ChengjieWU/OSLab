#include "types.h"
#include "printf.h"
#include "video.h"
#include "string.h"
#include "picture.h"
#include "time.h"
#include "keyboard.h"
#include "video_common.h"
#include "scan_code.h"

#define groudWidth SCR_WIDTH
#define groudHeight SCR_HEIGHT
#define groudDepth SCR_DEPTH

#define ballRadius 8
#define ballTop 300
#define ballLeft 300
#define boardHalfWidth 60
#define boardHalfHeight 8
#define brickRow 5
#define brickColumn 8
#define brickHalfWidth 30
#define brickHalfHeight 12
#define brickHalfSpace 8
#define brickLeft 400
#define brickTop 160
#define boardTop 540

#define ballspeedx 9
#define ballspeedy 9
#define speedSwitch 1
#define step 25

uint32_t game_time = 0;


bool brick[brickRow][brickColumn];
int ball_x, ball_y;
int board_x;
const int board_y = boardTop;
bool is_start;
int v_x, v_y;
int state;


int abs(int a)
{
	if (a >= 0) return a;
	else return -a;
}
int symbolInteger(int a)
{
	if (a > 0) return 1;
	else if (a == 0) return 0;
	else return -1;
}
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

/*--------------------------------------------------------------------------*/

void draw_rectangular(int x, int y, int p, int q, union Pixels c)
{
	uint8_t buffer[groudDepth] = {c.blue, c.green, c.red};
	int i, j;
	for (i = y; i < q; i++)
		for (j = x; j < p; j++)
			loadVideo(buffer, (i * groudWidth + j) * groudDepth, groudDepth);
}
void remove_rectangular(int x, int y, int p, int q)
{
	int size = (p - x) * groudDepth;
	int i;
	for (i = y; i < q; i++)
	{
		int position = (i * groudWidth + x) * groudDepth;
		loadVideo(gImage_Universe + position, position, size);
	}
}
void draw_ball()
{
	union Pixels c;
	c.blue = 0; c.green = 0; c.red = 255;
	draw_rectangular(ball_x - ballRadius, ball_y - ballRadius, ball_x + ballRadius, ball_y + ballRadius, c);
}
void remove_ball()
{
	remove_rectangular(ball_x - ballRadius, ball_y - ballRadius, ball_x + ballRadius, ball_y + ballRadius);
}
void draw_board()
{
	union Pixels c;
	c.blue = 0; c.green = 255; c.red = 0;
	draw_rectangular(board_x - boardHalfWidth, board_y - boardHalfHeight, board_x + boardHalfWidth, board_y + boardHalfHeight, c);
}
void remove_board()
{
	remove_rectangular(board_x - boardHalfWidth, board_y - boardHalfHeight, board_x + boardHalfWidth, board_y + boardHalfHeight);
}
void remove_a_brick(int i, int j)
{
	int sx = brickLeft - brickColumn * brickHalfWidth - (brickColumn - 1) * brickHalfSpace;
	int sy = brickTop - brickRow * brickHalfHeight - (brickRow - 1) * brickHalfSpace;
	int x = sx + j * 2 * (brickHalfWidth+brickHalfSpace);
	int y = sy + i * 2 * (brickHalfHeight+brickHalfSpace);
	remove_rectangular(x, y, x + 2 * brickHalfWidth, y + 2 * brickHalfHeight);
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


/*--------------------------------------------------------------------------*/
void init_display()
{
	fullVideo(gImage_Universe);
	draw_ball();
	draw_board();
	draw_brick();
}
void init_Game()
{
	is_start = false;
	state = 0;
	board_x = 400;
	ball_x = ballLeft;
	ball_y = ballTop;
	memset(brick, true, sizeof brick);
	v_x = ballspeedx;
	v_y = ballspeedy;
	//printf("%d", get_time);
	init_display();
}


/*--------------------------------------------------------------------------*/
bool collide_board()
{
	if (abs(board_y-ball_y) <= ballRadius+boardHalfHeight && abs(board_x-ball_x) <= ballRadius+boardHalfWidth) 
	{
		float gap = (float)abs(board_x - ball_x);
		float total = (float)boardHalfWidth;
		if (gap > total/3)
		{
			int safter = v_x - symbolInteger(board_x - ball_x);
			if (gap < 2*total/3 && abs(safter) > 5) v_x = safter;
			else 
			{	
				safter = v_x - 2 * symbolInteger(board_x - ball_x);
				if (gap >= 2*total/3 && abs(safter) > 5) v_x = safter;
			}
		}
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
			case K_Q: return -1;
			case K_ENTER:
			case K_SPACE: return 1;
			default: break;
		}
			
		uint32_t current = getTime()/speedSwitch;
		if (game_time != current)
		{
			game_time = current;
			if (ball_x + v_x < ballRadius || ball_x + v_x >= groudWidth - ballRadius) v_x = -v_x;
			if (ball_y + v_y < ballRadius) v_y = -v_y;
			if (ball_y + v_y >= groudHeight - ballRadius)
			{
				state = -1;
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
				state = 1;
				is_start = false;
			}
		}
	}
	return 0;
}



int game()
{
	printf("We are now in game!\n");
	init_Game();
	int key;
	while (true)
	{
		key = readKey();
		if (key == K_Q) break;
		else if (key == K_ENTER || key == K_SPACE) 
		{
			if (state != 0) {init_Game(); continue;}
			int ret = round();
			if (ret == -1) break;
			else if (ret == 1) continue;
			if (state == 1) fullVideo(gImage_SUCCESS);
			else if (state == -1) fullVideo(gImage_FAILURE);
			else return -1;
		}
	}
	return 0;
}