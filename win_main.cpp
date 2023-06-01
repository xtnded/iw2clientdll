#include "stdheader.h"
#include <Windows.h>

HINSTANCE hInst;

char sys_cmdline[1024];
char szAppData[MAX_PATH + 1];

static int(__stdcall *w_main)(HINSTANCE, HINSTANCE, LPSTR, int) = (int(__stdcall*)(HINSTANCE, HINSTANCE, LPSTR, int))0x466460;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	hInst = hInstance;
	strncpy_s(sys_cmdline, sizeof(sys_cmdline) - 1, lpCmdLine, _TRUNCATE);

	return w_main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}