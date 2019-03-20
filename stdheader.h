#pragma once

#include "stdafx.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdarg>

#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <ShellAPI.h>
#pragma comment(lib, "Shell32.lib")

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

#define MOD_NAME "Moto2"
#define MsgBox(x) MessageBoxA(0,x,0,0)

#if 0
typedef void(*Com_Printf_t)(const char*, ...);
Com_Printf_t Com_Printf = (Com_Printf_t)0x40A3DC;
#endif

/* dvar->flags */
#define DVAR_ARCHIVE		(1 << 0)	// 0x0001
#define DVAR_USERINFO		(1 << 1)	// 0x0002
#define DVAR_SERVERINFO		(1 << 2)	// 0x0004
#define DVAR_SYSTEMINFO		(1 << 3)	// 0x0008
#define DVAR_INIT		(1 << 4)	// 0x0010
#define DVAR_LATCH		(1 << 5)	// 0x0020
#define DVAR_ROM		(1 << 6)	// 0x0040
#define DVAR_CHEAT		(1 << 7)	// 0x0080
#define DVAR_DEVELOPER		(1 << 8)	// 0x0100
#define DVAR_SAVED		(1 << 9)	// 0x0200
#define DVAR_NORESTART		(1 << 10)	// 0x0400
#define DVAR_CHANGEABLE_RESET	(1 << 12)	// 0x1000
#define DVAR_EXTERNAL		(1 << 14)	// 0x4000
#define DVAR_AUTOEXEC		(1 << 15)	// 0x8000

enum dvarType_t
{
	DVAR_TYPE_INVALID = 0x0,
	DVAR_TYPE_BOOL = 0x1,
	DVAR_TYPE_FLOAT = 0x2,
	DVAR_TYPE_FLOAT_2 = 0x3,
	DVAR_TYPE_FLOAT_3 = 0x4,
	DVAR_TYPE_FLOAT_4 = 0x5,
	DVAR_TYPE_INT = 0x6,
	DVAR_TYPE_ENUM = 0x7,
	DVAR_TYPE_STRING = 0x8,
	DVAR_TYPE_COLOR = 0x9,
	DVAR_TYPE_INT64 = 0xA,
	DVAR_TYPE_LINEAR_COLOR_RGB = 0xB,
	DVAR_TYPE_COLOR_XYZ = 0xC,
	DVAR_TYPE_COUNT = 0xD,
};

union DvarLimits
{
	struct {
		int stringCount;
		const char **strings;
	} enumeration;

	struct {
		int min;
		int max;
	} integer;

	struct {
		float min;
		float max;
	} value, vector;

	struct {
		__int64 min;
		__int64 max;
	} integer64;
};

#pragma pack(push, 4)
struct vec4_t
{
	float x, y, z, w;
};
#pragma pack(pop)

union DvarValue
{
	bool enabled;
	int integer;
	unsigned int unsignedInt;
	__int64 integer64;
	unsigned __int64 unsignedInt64;
	float value;
	vec4_t vector;
	const char *string;
	char color[4];
};

#pragma pack(push, 4)
struct dvar_t
{
	const char *name;
	//const char *description;
	//int hash;
	unsigned int flags;
	dvarType_t type;
	bool modified;
	DvarValue current;
	DvarValue latched;
	DvarValue reset;
	DvarLimits domain;
	dvar_t *hashNext;
	int unknown3;
};
#pragma pack(pop)

static char    * __cdecl va(const char *format, ...) {
	va_list argptr;
#define MAX_VA_STRING   32000
	static char temp_buffer[MAX_VA_STRING];
	static char string[MAX_VA_STRING];      // in case va is called by nested functions
	static int index = 0;
	char    *buf;
	int len;


	va_start(argptr, format);
	vsprintf(temp_buffer, format, argptr);
	va_end(argptr);

	if ((len = strlen(temp_buffer)) >= MAX_VA_STRING) {
		return string;
		//Com_Error( ERR_DROP, "Attempted to overrun string in call to va()\n" );
	}

	if (len + index >= MAX_VA_STRING - 1) {
		index = 0;
	}

	buf = &string[index];
	memcpy(buf, temp_buffer, len + 1);

	index += len + 1;

	return buf;
}

//static void(*Com_Printf)(const char*, ...) = (void(*)(const char*, ...))0x40A3DC;
template <typename T, typename ... Ts>
T call(size_t addr, Ts ... ts) {
	T(*f)(...);
	*(T*)&f = (T)addr;
	return f(ts...);
}

static void(*Com_PrintMessage)(int, const char*) = (void(*)(int, const char*))0x431D10;
template <typename ... Ts>
void Com_Printf(const char* fmt, Ts ... ts) {
#if 0
	//char out[256];
	//sprintf(out, "Test: %f", CODPATCH);
	//MessageBox(NULL, out, "MDLL", MB_OK);
	if (CODPATCH == 5)
		call<void*, const char*, Ts...>(0x437C00, fmt, ts...);
	else if (CODPATCH == 1)
		call<void*, const char*, Ts...>(0x4357B0, fmt, ts...);
#endif
	Com_PrintMessage(0, va(fmt, ts...));
}
static void(*SV_SendServerCommand)(int, const char*, ...) = (void(*)(int, const char*, ...))0x045A670;
static void(*Cbuf_AddText)(const char*) = (void(*)(const char*))0x40AD22;
static void(*Cmd_AddCommand)(const char*, void*) = (void(*)(const char*, void*))0x4212F0;
static dvar_t*(*Dvar_GetVariantString)(const char*) = (dvar_t*(*)(const char*))0x4373A0;
//static void(*Dvar_SetVariant)(dvar_t*,void*,int) = (dvar_t(*)(void*,void*,int))0x438900;
//^BUGGED ATM
static dvar_t*(*Dvar_RegisterString)(const char*, const char*, unsigned short) = (dvar_t*(*)(const char*, const char*, unsigned short))0x437DE0;
static void(*Dvar_SetFromStringByName)(const char*, const char*) = (void(*)(const char*, const char*))0x439150;
static void Dvar_SetString(const char *_dvar, const char *strval)
{
	void *dvar = Dvar_GetVariantString(_dvar);
	if (!dvar)
		Dvar_RegisterString(_dvar, strval, 4160);
}

static
int Sys_IsAdmin() {
	int b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return(b);
}

static
int Sys_GetModulePathInfo(HMODULE module, char **path, char **filename, char **extension) {
	int sep = '/';
	static char szFileName[MAX_PATH + 1];
	if (path)
		*path = NULL;
	if (filename)
		*filename = NULL;
	if (extension)
		*extension = NULL;

	GetModuleFileNameA(module, szFileName, MAX_PATH);

	char *fn = strrchr(szFileName, sep);
	if (fn == nullptr) {
		sep = '\\';
		fn = strrchr(szFileName, sep);
	}
	if (fn != NULL) {
		*fn++ = 0;

		char *ext = strrchr(fn, '.');

		if (ext != NULL) {
			if (fn != ext) {
				if (extension)
					*ext++ = 0;
				if (path)
					*path = szFileName;
				if (filename)
					*filename = fn;
				if (extension)
					*extension = ext;
			}
		}
	}
	return sep;
}

static void(*Com_Quit_f)() = (void(*)())0x435D80;

static
bool Sys_ElevateProgram(char *arg3, bool restart) {
	if (Sys_IsAdmin() && !restart)
		return false; //we already are admin

	char *fn;

	Sys_GetModulePathInfo(NULL, NULL, &fn, NULL);

	char *arg;
#ifdef UPDATE_EXE
	arg = arg3;
#else
	if (!arg3)
		arg = (char*)"allowdupe";
	else
		arg = va((char*)"allowdupe %s", arg3);
#endif
	DWORD err = ERROR_SUCCESS;
	ShellExecuteA(NULL, "runas", fn, arg, ".", SW_SHOWNORMAL | SW_SHOW);
	if ((err = GetLastError()) != ERROR_SUCCESS)
		if (err != ERROR_CANCELLED);
#ifdef UPDATE_EXE
	exit(0);
#else
	Com_Quit_f();
#endif
	return true;
	//PostQuitMessage(0);
}

#if 0
static void __call(unsigned int off, unsigned int loc) {
#ifdef _WIN32
	DWORD tmp;
	VirtualProtect((void*)off, 5, PAGE_EXECUTE_READWRITE, &tmp);
#endif
	int foffset = loc - (off + 5);
	memcpy((void *)(off + 1), &foffset, 4);
	FlushInstructionCache(GetCurrentProcess(), (void*)off, 5);
#ifdef _WIN32
	VirtualProtect((void*)off, 5, tmp, &tmp);
#endif
}
#endif