#include "types.h"
#include "printf.h"
#include "string.h"
#include "keyboard.h"
#include "proc.h"

char strbuf[50];

void shell_main()
{
	memset(strbuf, 0, sizeof strbuf);
	getline(strbuf);
	exit();
}
