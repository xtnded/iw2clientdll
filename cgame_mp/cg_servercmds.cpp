#include "../qcommon/qcommon.h"

//char*(*CG_Argv)(int) = (char*(*)(int))0x4BF890;

char **cg_argv = (char**)0xB17A80;
int *cg_argc = (int*)0xB1A480;

unsigned int CG_Argc()
{
	return *cg_argc;
}

char *CG_Argv(int i)
{
	if (i >= CG_Argc())
		return (char*)"";
	return cg_argv[i];
}

void CG_ServerCommand(void)
{
	void(*o)(void) = (void(*)(void))0x4D1B80;
	char *arg = CG_Argv(0);

	switch (*arg)
	{
	case 0:
		return;

	case 'v': //setclientdvar
		char *dvarName = CG_Argv(1);
		char *dvarValue = CG_Argv(2);
		if (!_stricmp(dvarName, "name"))
		{
			//allow it for now
			//Com_Printf("server tried to change your name to '%s'\n", dvarValue);
			//return;
		}
		break;
	}

	o();
}