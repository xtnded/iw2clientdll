#include <Windows.h>

#include "../util/memutil/memutil.h"
#include "../util/storage/storage.h"
#include "../qcommon/qcommon.h"
#include "../detours/detours.h"

extern dvar_t* cl_imguiEnabled;
extern bool imgui_enabled(dvar_t*);

void Main_UnprotectModule(HMODULE hModule)
{
	PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + header->e_lfanew);

	// unprotect the entire PE image
	SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
	DWORD oldProtect;
	VirtualProtect((LPVOID)hModule, size, PAGE_EXECUTE_READWRITE, &oldProtect);
}

int improper_closed()
{
	return 1;
}

void checker()
{
	MessageBoxA(0, 0, 0, 0);
}

static int ddraw_exists = 0;
static const char *custom_gl_driver = "gfx_gl_mp_x86_s.dll";
__declspec(naked) void hook_gfx_driver()
{
	__asm
	{
		mov esi, eax

		mov eax, ddraw_exists
		test eax, eax

		jz def
		mov eax, custom_gl_driver
		push eax
		//FF 15 BC 31 59 00
		__emit 0xff
		__emit 0x15
		__emit 0xbc
		__emit 0x31
		__emit 0x59
		__emit 0x0

		jmp back

		def:

		call checker
		push esi
		__emit 0xff
		__emit 0x15
		__emit 0xbc
		__emit 0x31
		__emit 0x59
		__emit 0x0


		back:
		mov ebx, 0x464EC9
		jmp ebx
	}
}

bool imguiEnabled = false;
bool preventMouseGrab = false;

BOOL(__stdcall*oSetCursorPos)(int, int);
BOOL __stdcall hSetCursorPos(int x, int y)
{
	if (imguiEnabled|| preventMouseGrab)
		return TRUE;
	return SetCursorPos(x, y);
}

POINT cachedPt;

BOOL(__stdcall*oGetCursorPos)(LPPOINT);
BOOL __stdcall hGetCursorPos(LPPOINT pt)
{
	if (imguiEnabled|| preventMouseGrab)
	{
		pt->x = cachedPt.x;
		pt->y = cachedPt.y;
		return TRUE;
	}
	auto result = GetCursorPos(pt);
	cachedPt.x = pt->x;
	cachedPt.y = pt->y;
	return result;
}

HWND gameHwnd = NULL;

void SetWndCapture(bool f)
{
	if (!gameHwnd)return;
	if (f)
	{
		//XUNLOCK((void*)0x464750, 1);
		//*(unsigned char*)0x464750 = 0xc3; //retn
		ReleaseCapture();
	}
	else
	{
		SetCapture(gameHwnd);
#if 0
		//sub_464750
		void(*uh)();
		*(int*)&uh = 0x464750;
		uh();
#endif
	}
	ShowCursor(f ? 1 : 0);
}


// eating a lot of memory probably but cba to think on a better check
void Cmd_ImGui_f()
{
	dvar_t* r_windowed = Dvar_RegisterBool("r_windowed", false, CVAR_ARCHIVE);
	if (!r_windowed->current.enabled) {
		imguiEnabled ^= (const int)imgui_enabled(cl_imguiEnabled);
		//escape
		SetWndCapture(imguiEnabled);
	}
	else {
		Com_Printf("In order to use imgui, you need to disable windowed mode!\n`");
	}

	//client needs to do /vid_restart in order to make code go through the if once again
}

LRESULT CALLBACK MyWndProc(
	HWND   hwnd,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{

	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	extern bool r_inited;
	if (r_inited && imguiEnabled)
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_CREATE:
		gameHwnd = hwnd;
		SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_SIZEBOX);
		break;

	case WM_SIZE:
	{
		int nw = LOWORD(lParam);
		int nh = HIWORD(lParam);
		if(wParam != SIZE_MINIMIZED && r_inited)
		{
			void R_Resize(int w, int h);
			R_Resize(nw, nh);
		}
	} break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			if (imguiEnabled)
			{
				Cmd_ImGui_f();
				return 0;
			}
			break;
		default:
			//Com_Printf("key = %d\n", wParam);
			break;
		}
	} break;
#if 0
	default:
		Com_Printf("msg = %d\n", uMsg);
		break;
#endif
	case 32:
	case 513:
	case 514:
	case 516:
	case 517:
	case 123:
	case 519:
	case 520:
	case 257:
	case 258:
	case WM_MOUSEFIRST:
		if (imguiEnabled)
			return 0;
		break;
	}

	LRESULT(CALLBACK *o)(HWND, UINT, WPARAM, LPARAM);
	*(int*)&o = 0x468DB0;
	return o(hwnd, uMsg, wParam, lParam);
}

typedef struct
{
	char server[5][64];
} updateservers_t;

updateservers_t *updateservers = (updateservers_t*)0x966C08;
static const char *updateServerStr = "xtnded.org";

void CL_ResolveUpdateServers()
{

	for (int i = 0; i < 5; ++i)
	{
		snprintf(updateservers->server[i], sizeof(updateservers->server[i]), "%s", updateServerStr);
	}

	void(*o)(void) = (void(*)(void))0x4B4E20;
	o();

	//at cl_init it's too late meh
	//i realized i had [5][64] backwards and kept crashing, and fixed bunch of stuff didnt need fixing, but this way it works aswell so oh well
}

void applyHooks()
{
	__try
	{
		//if (*(int*)5905726 != 840977744)
			//return;
		char *str = (char*)0x5A1D3C;
		if (strcmp(str, "CoD2 MP"))
			return;
	}
	__except (1)
	{
		return;
	}
	//TerminateProcess(GetCurrentProcess(), 0);

	HMODULE hModule;
	if (SUCCEEDED(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)applyHooks, &hModule)))
	{
		Main_UnprotectModule(hModule);
	}

#if 0
	oSetCursorPos = (BOOL(__stdcall*)(int, int)) \
		DetourFunction((LPBYTE)SetCursorPos, (LPBYTE)hSetCursorPos);
	oGetCursorPos = (BOOL(__stdcall*)(LPPOINT)) \
		DetourFunction((LPBYTE)GetCursorPos, (LPBYTE)hGetCursorPos);
#endif
	/*
	Direction Type Address        Text                   
--------- ---- -------        ----                   
Up        p    sub_464820+7   call    ds:GetCursorPos
Up        p    sub_4649C0+15  call    ds:GetCursorPos
Up        r    sub_464820+7   call    ds:GetCursorPos
Up        r    sub_4649C0+15  call    ds:GetCursorPos
Up        r    .text:00547DE0 jmp     ds:GetCursorPos*/
#define __ffcall(addr, func) \
XUNLOCK((void*)addr,6); \
*(unsigned char*)(addr) = 0xe8; \
__call(addr, func); \
*(unsigned char*)(addr + 5) = 0x90;

	__ffcall(0x464827, (int)hGetCursorPos);
	__ffcall(0x4649D5, (int)hGetCursorPos);

	/*
	Direction Type Address        Text                   
--------- ---- -------        ----                   
Up        p    sub_464750+4F  call    ds:SetCursorPos
Up        p    sub_464820+1B  call    ds:SetCursorPos
Up        p    sub_4649C0+29  call    ds:SetCursorPos
Up        r    sub_464750+4F  call    ds:SetCursorPos
Up        r    sub_464820+1B  call    ds:SetCursorPos
Up        r    sub_4649C0+29  call    ds:SetCursorPos
          r    .text:00547DE6 jmp     ds:SetCursorPos*/

	__ffcall(0x46479F, (int)hSetCursorPos);
	__ffcall(0x46483B, (int)hSetCursorPos);
	__ffcall(0x4649E9, (int)hSetCursorPos);



#define CLIENT_UPDATE_PORT (25561)
	*(unsigned int*)(0x4B4F10 + 1) = CLIENT_UPDATE_PORT;
	__call(0x41162F, (int)CL_ResolveUpdateServers);
	void CL_UpdateInfoPacket(netadr_t);
	__call(0x40EF9C, (int)CL_UpdateInfoPacket);
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	__call(0x57DCD3, (int)WinMain);


	/* doesn't really match up to q3 and its quite inlined changed mhm */
	void CL_Frame();
	__call(0x43506A, (int)CL_Frame);

	void CL_Init( void );
	__call(0x4348B9, (int)CL_Init);

	void patch_opcode_loadlibrary(void);
	patch_opcode_loadlibrary();

	//void patch_opcode_glbindtexture(void);
	//patch_opcode_glbindtexture();

	void CG_ServerCommand(void);
	__call(0x4D216F, (int)CG_ServerCommand);

	//patch mic crash issue cod2
	XUNLOCK((void*)0x005B375C, 1);
	XUNLOCK((void*)0x005B372D, 1);
	*(unsigned char*)0x005B375C = 'x';
	*(unsigned char*)0x005B372D = 'x';

	//ddraw_exists = storage::file_exists(custom_gl_driver) ? 1 : 0;
	//ddraw_exists = 1;
	//__jmp(0x464EC0, (int)hook_gfx_driver);

	XUNLOCK((void*)0x4663D1, 10);
	*(int*)(0x4663D1 + 4) = (int)MyWndProc;

#define PATCH_PUSH_STRING_PTR_VALUE(offset, new_str) \
	XUNLOCK((void*)offset, 10); \
	*(const char **)(offset + 1) = new_str;

	PATCH_PUSH_STRING_PTR_VALUE(0x43477C, "1.4");
	PATCH_PUSH_STRING_PTR_VALUE(0x434701, "1.4");
	PATCH_PUSH_STRING_PTR_VALUE(0x4064C1, "1.4");
	PATCH_PUSH_STRING_PTR_VALUE(0x4346DE, __DATE__ " " __TIME__);
	PATCH_PUSH_STRING_PTR_VALUE(0x4346E3, "pc_1.4_1_0");
	PATCH_PUSH_STRING_PTR_VALUE(0x407180, __DATE__ " " __TIME__);
	PATCH_PUSH_STRING_PTR_VALUE(0x407185, "pc_1.4_1_0");

	//nop splash screen
	__nop(0x466555, 0x466555 + 5);
	__nop(0x046664A, 0x046664A + 2);
	
	//fix for the blackscreen bug by php
	__nop(0x4B9569, 2);

	__jmp(0x466270, (int)improper_closed);

	//void CG_DrawFPS();
	//__call(0x4C774E, (int)CG_DrawFPS);
}
