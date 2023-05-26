#include "stdheader.h"
#include "cg_local.h"
#include <string.h>
#include "chakracore.h"

// example: /cg_abc 1 2.2 three
// output: argc=4 (int, float, string) = (1, 2.2, "three")

void CG_abc_f() {
	int a = atoi(CG_Argv(1));
	float b = atof(CG_Argv(2));
	char *c = CG_Argv(3);
	//Com_Printf("argc=%d (int, float, string) = (%d, %f, \"%s\")", CG_Argc(), a, b, c);
}

// example: /js console.log('^3hello js')
// currently " is not supported, use '

void CG_js_f() {
	// todo: access original console buffer so we can rip " out of it etc.
	std::string line;
	
	for (int i = 1; i < CG_Argc(); i++) {
		line += CG_Argv(i);
		line += " ";
	}
	//Com_Printf("line: %s\n", line.c_str());

	// wrap the code up for eval_repl so we can print the output, use try/catch etc., makes it easier than filthy JS api
	std::string replstring;
	replstring += "eval_repl(\"";
	replstring += line;
	replstring += "\");";
	chakracore_eval(replstring.c_str());
	chakracore_eval(line.c_str());
}

void CG_InitConsoleCommands(void) {
	void Cmd_ImGui_f();
	Cmd_AddCommand("imgui", Cmd_ImGui_f);
	Cmd_AddCommand("cg_abc", CG_abc_f);
	Cmd_AddCommand("js", CG_js_f);
}