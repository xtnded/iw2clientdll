#include "../qcommon/qcommon.h"
#include "cg_local.h"
#include <string.h>

void CG_InitConsoleCommands(void) {
	void Cmd_ImGui_f();
	Cmd_AddCommand("imgui", Cmd_ImGui_f);
}