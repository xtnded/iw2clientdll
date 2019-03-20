#include "stdheader.h"

#pragma comment(lib, "detours.lib")
#include "detours.h"

__int64 FileSize(std::string name)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesExA(name.c_str(), GetFileExInfoStandard, &fad))
		return -1; // error condition, could call GetLastError to find out more
	LARGE_INTEGER size;
	size.HighPart = fad.nFileSizeHigh;
	size.LowPart = fad.nFileSizeLow;
	return size.QuadPart;
}

HMODULE(WINAPI *orig_LoadLibraryA)(LPCSTR lpFileName);
HMODULE WINAPI hLoadLibraryA(LPSTR lpFileName) {
	HMODULE hModule = orig_LoadLibraryA(lpFileName);
	DWORD pBase = (DWORD)GetModuleHandleA( lpFileName );
	if (!pBase)
		return hModule;

	void Main_UnprotectModule(HMODULE hModule);
	Main_UnprotectModule(hModule);

	std::string lowercase;
	for (auto *c = lpFileName; *c; c++)
		lowercase.push_back(tolower(*c));

	if (!strcmp(lpFileName, "gfx_d3d_mp_x86_s.dll") || !strcmp(lpFileName, "gfx_gl_mp_x86_s.dll"))
	{
		extern DWORD gfx_dll_mp;
		gfx_dll_mp = pBase;
		void patch_gfx_dll();
		patch_gfx_dll();
	}

	if (lowercase.find("opengl32") != std::string::npos)
	{
		//void patch_opcode_glbindtexture(HMODULE);
		//patch_opcode_glbindtexture(hModule);
	}
#if 0
	if (lowercase == "d3d9.dll")
	{
		void applyHooks();
		applyHooks();
	}
#endif
	char *str = (char*)0x5A1D3C;
	//MessageBoxA(0, va("str = %s, lib = %s\n",str,lpFileName), 0, 0);
	//MsgBox(lpFileName);
	//Com_Printf("^2dll name = %s\n", lowercase.c_str());

	return hModule;
}
#if 0
HWND
(__stdcall *oCreateWindowExA)(
	DWORD dwExStyle,
	LPCSTR lpClassName,
	LPCSTR lpWindowName,
	DWORD dwStyle,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam);

HWND __stdcall hCreateWindowExA(DWORD dwExStyle,
	LPCSTR lpClassName,
	LPCSTR lpWindowName,
	DWORD dwStyle,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam)
{
	HWND wnd = oCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	return wnd;
}

void hookCreateWindowExA(void)
{

	/*
	int from = 0x4634AC;
	DWORD tmp;
	VirtualProtect((void*)from, 6, PAGE_EXECUTE_READWRITE, &tmp);
	*(BYTE*)(from) = 0xbf;
	*(int*)(from + 1) = (int)hLoadLibraryA;
	*(BYTE*)(from + 5) = 0x90;
	VirtualProtect((void*)from, 6, tmp, &tmp);
	*/


	orig_LoadLibraryA = (struct HINSTANCE__ *(__stdcall*)(const char*)) \
		DetourFunction((LPBYTE)LoadLibraryA, (LPBYTE)hLoadLibraryA);
}
#endif

void patch_opcode_loadlibrary(void)
{

	/*
	int from = 0x4634AC;
	DWORD tmp;
	VirtualProtect((void*)from, 6, PAGE_EXECUTE_READWRITE, &tmp);
	*(BYTE*)(from) = 0xbf;
	*(int*)(from + 1) = (int)hLoadLibraryA;
	*(BYTE*)(from + 5) = 0x90;
	VirtualProtect((void*)from, 6, tmp, &tmp);
	*/


	orig_LoadLibraryA = (struct HINSTANCE__ *(__stdcall*)(const char*)) \
	DetourFunction((LPBYTE)LoadLibraryA, (LPBYTE)hLoadLibraryA);
}