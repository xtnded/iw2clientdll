#include "../qcommon/qcommon.h"
#include "../util/memutil/memutil.h"

dvar_t* com_developer;

void Com_InitDvars()
{
	com_developer = Dvar_RegisterFloat("developer", 0, 0, 2, DVAR_CHANGEABLE_RESET);
}

void Com_Init_Try_Block_Function(char* commandLine)
{
	Com_InitDvars();
}