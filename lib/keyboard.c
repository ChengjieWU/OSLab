#include "keyboard.h"
#include "printf.h"

int getkey()
{
	int key = K_NULL;
	while (key == K_NULL || (key & 0x80) != 0) key = readKey();
	return key;
}

/* only lowercase letters are allowed, and return length read. */
int getline(char *strbuf)
{
	int bufptr = 0;
	int key = K_NULL;
	while (key != K_ENTER)
	{
		key = getkey();
		if (key == K_ENTER) strbuf[bufptr++] = '\0';
		else
		{
			char kt = transkey(key);
			if (kt - 'a' < 0 || kt - 'a' >= 26) continue;
			printf("%c", kt);
			strbuf[bufptr++] = kt;
		}
	}
	printf("\n");
	return bufptr;
}

/* translate scan code into actual character */
char transkey(int key)
{
	switch (key) 
	{
		case K_A: return 'a';
		case K_B: return 'b';
		case K_C: return 'c';
		case K_D: return 'd';
		case K_E: return 'e';
		case K_F: return 'f';
		case K_G: return 'g';
		case K_H: return 'h';
		case K_I: return 'i';
		case K_J: return 'j';
		case K_K: return 'k';
		case K_L: return 'l';
		case K_M: return 'm';
		case K_N: return 'n';
		case K_O: return 'o';
		case K_P: return 'p';
		case K_Q: return 'q';
		case K_R: return 'r';
		case K_S: return 's';
		case K_T: return 't';
		case K_U: return 'u';
		case K_V: return 'v';
		case K_W: return 'w';
		case K_X: return 'x';
		case K_Y: return 'y';
		case K_Z: return 'z';
		case K_ENTER: return '\0';
	}
	return '\0';
}
