#include "stdheader.h"
#include "gl33.h"
#include "imgui.h"
#include "cg_public.h"
#include "chakracore.h"
#include "cg_local.h"

dvar_t *con_restricted = (dvar_t*)0x5E132C;
cvar_t* cl_imguiEnabled;
int *cls_keyCatchers = (int*)0x96B654;
#define KEYCATCH_CONSOLE (1)

bool imgui_enabled(cvar_t*);

void Cbuf_AddText_(const char *s)
{
	__asm
	{
		mov eax, s
		lea edx, Cbuf_AddText
		call edx
	}
}

void handleImGuiWindows()
{
	static char serverCommand[4096] = { 0 };

	if (ImGui::Begin("haha"))
	{

		ImGui::InputText("server cmd", serverCommand, sizeof(serverCommand));
		if (ImGui::Button("send"))
		{
			Cbuf_AddText(serverCommand);
		}
		ImGui::End();
	}
}

void SetWndCapture(bool);
int *renderThreadId = (int*)0xD528D8;
void *devFont = (void*)0x14EB4FC;
float *font_xscale = (float*)0xC94C08;
float *font_yscale = (float*)0xC94C0C;
void *consoleFont = (void*)0x966C04;
//our own kind of cl_frame to handle things each frame / tick
void CL_Frame()
{
	void(*o)() = (void(*)())0x040F850;
	o();

	imgui_enabled(cl_imguiEnabled);

	extern bool preventMouseGrab;
	bool prev = preventMouseGrab;
	preventMouseGrab = (*cls_keyCatchers & KEYCATCH_CONSOLE) == KEYCATCH_CONSOLE;
	if (prev)
	{
		if (!preventMouseGrab)
		{
			//closing console
			SetWndCapture(false);
		}
	}
	else
	{
		if(preventMouseGrab)
			SetWndCapture(true);
	}
	if (!CL_DrawText)
	{
		CL_DrawText = (void(*)(const char*,int, void*,float,float,float,float,float*,int)) (*(void**)0x68A31C);
	}
	float clr[] = { 255,255,255,255 };
	//CL_DrawText("CoD2 1.4 Developer Edition", 1000, (int)consoleFont, 100, 100, *font_xscale, *font_yscale, clr, 0);
	//CL_DrawText("AAAAAAAAAAAAAAAA", 0x7FFFFFFF, consoleFont, 1.f, 1.f, 1065353216, 1065353216, (float*)0x59BCA8, 0);
	//Com_Printf("frame!\n");
}

// eating a lot of memory probably but cba to think on a better check
bool imgui_enabled(cvar_t *cl_imguiEnabled) {
	return cl_imguiEnabled->boolean == true;
}

int Com_PrintString(const char *str) {
	Com_Printf("%s", str);
	return 0;
}

void CL_UpdateInfoPacket(netadr_t adr)
{
	//this works, but UDP unreliable bs and it's loading the game and it wont do the udp packets, so scrap this, and using http file now yay, TODO
#if 0
	for (int i = 0; i < CG_Argc(); ++i)
	{
		Com_Printf("^6%d: %s", i, CG_Argv(i));

		//MessageBoxA(NULL, CG_Argv(i), "", 0);
	}
#endif
}

void CL_Init( void )
{
	void(*o)(void) = (void(*)(void))0x411650;

	o();

	Dvar_SetFromStringByName("moto", "yes");
	cl_imguiEnabled = Cvar_RegisterBool("cl_imguiEnabled", false, CVAR_ARCHIVE); // by default it's false because not everyone wants to use an imgui, Flag set to Archive because why not
	Com_Printf(MOD_NAME " loaded!\n");

	//cl_inited = true;

	CG_InitConsoleCommands();

	chakracore_prepare(Com_PrintString);
	chakracore_eval("console.log(\"^3Loaded ChakraCore\");");
	const char *eval_repl =
		"eval_repl = function(code) {"\
		"	try {					 "\
		"		var ret = eval(code);"\
		"		console.log(ret)	 "\
		"	}						 "\
		"	catch (e) {				 "\
		"		console.log(e);		 "\
		"	}						 "\
		"};							 ";
	chakracore_eval(eval_repl);
}