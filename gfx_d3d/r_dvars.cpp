#include "../qcommon/qcommon.h"
#include "../util/memutil/memutil.h"

dvar_t* r_fullscreen = (dvar_t*)GFX_OFF(0x101CE60C);

void R_RegisterDvars(void)
{
	Com_Printf("----- R_RegisterDvars Initialization -----\n");
	void(*oR_RegisterDvars)(void) = (void(*)(void))GFX_OFF(0x1000AAF0);
	oR_RegisterDvars();

	Com_Printf("----- R_RegisterDvars Initialization Completed -----\n");
}