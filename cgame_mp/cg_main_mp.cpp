#include "../qcommon/qcommon.h"
#include "../util/memutil/memutil.h"
#include "cg_draw_mp.h"
#include "cg_main_mp.h"

dvar_t* cg_xui_fps;
dvar_t* cg_xui_fps_x;
dvar_t* cg_xui_fps_y;

void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum)
{
	void(*oCG_Init)(int, int, int) = (void(*)(int, int, int))0x4C0DD0;
	oCG_Init(serverMessageNum, serverCommandSequence, clientNum);

	Com_Printf("----- Client Game Initialization -----\n");

	cg_xui_fps = Dvar_RegisterBool("cg_xui_fps", true, DVAR_ARCHIVE);
	cg_xui_fps_x = Dvar_RegisterFloat("cg_xui_fps_x", 1850, 0, 9999, DVAR_ARCHIVE);
	cg_xui_fps_y = Dvar_RegisterFloat("cg_xui_fps_y", 8, 0, 9999, DVAR_ARCHIVE);

	__call(0x4C774E, (int)CG_DrawFPS);

	Com_Printf("----- Client Game Initialization Completed -----\n");
}