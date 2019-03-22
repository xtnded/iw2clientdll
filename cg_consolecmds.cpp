#include "stdheader.h"
#include "cg_local.h"

// example: /cg_abc 1 2.2 three
// output: argc=4 (int, float, string) = (1, 2.2, "three")

void CG_abc_f() {
	int a = atoi(CG_Argv(1));
	float b = atof(CG_Argv(2));
	char *c = CG_Argv(3);
	Com_Printf("argc=%d (int, float, string) = (%d, %f, \"%s\")", CG_Argc(), a, b, c);
}

void CG_InitConsoleCommands(void) {

	void Cmd_ImGui_f();
	Cmd_AddCommand("imgui", Cmd_ImGui_f);
	Cmd_AddCommand("cg_abc", CG_abc_f);
}