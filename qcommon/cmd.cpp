#include "qcommon.h"

static int cmd_argc;
static char* cmd_argv[MAX_STRING_TOKENS];        // points into cmd_tokenized\

/*
============
Cmd_Argc
============
*/
int Cmd_Argc(void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
const char* Cmd_Argv(int arg)
{
	if (arg >= cmd_argc)
	{
		return "";
	}

	return cmd_argv[arg];
}