#include "types.h"
#include "printf.h"
#include "string.h"
#include "keyboard.h"
#include "proc.h"
#include "filesystem.h"

#define strLength 20
#define argNum 10
unsigned char placeholder[1440000 * 3];

void print_prompt()
{
	printf("admin@myOS:/$ ");
}

int read_command(char *command, char arg[][strLength])
{
	char strbuf[strLength * argNum];
	getline(strbuf);
	if (strbuf[0] == '\0') return -1;
	int i = 0;
	while (strbuf[i] != '\0' && strbuf[i] != ' ')
	{
		command[i] = strbuf[i];
		i++;
	}
	command[i++] = '\0';
	
	int arg_count = 0;
	int arg_position = 0;
	while (strbuf[i] != '\0')
	{
		if (strbuf[i] == ' ')
		{
			arg[arg_count][arg_position] = '\0';
			arg_count += 1;
			arg_position = 0;
			while (strbuf[i] == ' ') i++;
			i -= 1;
		}
		else arg[arg_count][arg_position++] = strbuf[i];
		i++;
	}
	arg[arg_count][arg_position] = '\0';
	arg_count += 1;
	return arg_count;
}

int cmd_ls(int argc, char argv[][strLength])
{
	int i;
	int a = 0, l = 0, h = 0;
	for (i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-a\0") == 0) a = 1;
		else if (strcmp(argv[i], "-l\0") == 0) l = 1;
		else if (strcmp(argv[i], "-h\0") == 0) h = 1;
	}
	return ls(a, l, h);
}


void shell_main()
{
	//memset(no_use, 1, sizeof no_use);
	printf("Welcome to myShell!\n");
	char cmd[strLength];
	char arg[argNum][strLength];
	while (true) 
	{
		print_prompt();
		int argc = read_command(cmd, arg);
		if (!(argc >= 0 && argc <= argNum)) printf("Illegal command!\n");
		else if (strcmp(cmd, "ls") == 0) cmd_ls(argc, arg);
		else
		{
			char tmp[strLength]; tmp[0] = cmd[0]; tmp[1] = cmd[1]; tmp[2] = '\0';
			if (strcmp(tmp, "./\0") == 0)
			{
				int i = 2;
				while (cmd[i] != '\0') {tmp[i - 2] = cmd[i]; i++;}
				tmp[i - 2] = '\0';
				if (fork() == 0) exec(tmp);
				else drop_exec();
			}
		}
	}
	exit();
}
