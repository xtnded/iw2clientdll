#include "stdheader.h"
#include "timeapi.h"
#include "stdafx.h"

Cvar_RegisterBool_t Cvar_RegisterBool = (Cvar_RegisterBool_t)0x438040;
Cvar_RegisterFloat_t Cvar_RegisterFloat = (Cvar_RegisterFloat_t)0x438100;
Dvar_RegisterString_t Dvar_RegisterString = (Dvar_RegisterString_t)0x437DE0;
Dvar_SetFromStringByName_t Dvar_SetFromStringByName = (Dvar_SetFromStringByName_t)0x439150;
Dvar_Set_t Dvar_Set = (Dvar_Set_t)0x439E90;
Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x4212F0;
Com_PrintMessage_t Com_PrintMessage = (Com_PrintMessage_t)0x431D10;
CL_DrawString_t CL_DrawString = (CL_DrawString_t)0x4129F0;
Com_Error_t Com_Error = (Com_Error_t)0x4324C0;
Dvar_GetVariantString_t Dvar_GetVariantString = (Dvar_GetVariantString_t)0x4373A0;
Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x420AD0; //0x40AD22
CL_DrawText_t CL_DrawText = (CL_DrawText_t)0x68A31C;
SV_SendServerCommand_t SV_SendServerCommand = (SV_SendServerCommand_t)0x045A670;
Com_Quit_f_t Com_Quit_f = (Com_Quit_f_t)0x4326C0;//0x435D80
Com_Printf_t Com_Printf = (Com_Printf_t)0x431EE0;
Dvar_SetVariant_t Dvar_SetVariant = (Dvar_SetVariant_t)0x437090;
SCR_DrawSmallStringExt_t SCR_DrawSmallStringExt = (SCR_DrawSmallStringExt_t)0x4146A0;
CG_DrawBigDevStringColor_t CG_DrawBigDevStringColor = (CG_DrawBigDevStringColor_t)0; //0x4C2600 crashing reason is unknown
R_DrawText_t R_DrawText = (R_DrawText_t)GFX_OFF(0x1000C030);

char* __cdecl va(const char* format, ...) {
	va_list argptr;
#define MAX_VA_STRING   32000
	static char temp_buffer[MAX_VA_STRING];
	static char string[MAX_VA_STRING];      // in case va is called by nested functions
	static int index = 0;
	char* buf;
	int len;

	va_start(argptr, format);
	vsnprintf(temp_buffer, sizeof(temp_buffer), format, argptr);
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

void Dvar_SetString(const char* _dvar, const char* strval) {
	void* dvar = Dvar_GetVariantString(_dvar);
	if (!dvar)
		Dvar_RegisterString(_dvar, strval, 4160);
}

typedef struct {
	drawSurf_t	drawSurfs[MAX_DRAWSURFS];
	dlight_t	dlights[MAX_DLIGHTS];
	trRefEntity_t	entities[MAX_ENTITIES];
	srfPoly_t* polys;//[MAX_POLYS];
	polyVert_t* polyVerts;//[MAX_POLYVERTS];
	renderCommandList_t	commands;
} backEndData_t;
backEndData_t* backEndData[SMP_FRAMES];

/*
============
R_GetCommandBuffer

make sure there is enough command space, waiting on the
render thread if needed.
============
*/

void* R_GetCommandBuffer(int bytes) {
	renderCommandList_t* cmdList;

	cmdList = &backEndData[1]->commands;

	if (cmdList == NULL) {
		// Handle the case where cmdList is NULL
		// Print an error message, throw an exception, or take appropriate action
		// Example:
		Com_Error(ERR_FATAL, "Error: cmdList is NULL.");
		return NULL; // or throw an exception, exit the function, etc.
	}

	// always leave room for the end of list command
	if (cmdList->used + bytes + 4 > MAX_RENDER_COMMANDS) {
		if (bytes > MAX_RENDER_COMMANDS - 4) {
			Com_Error(ERR_FATAL, "R_GetCommandBuffer: bad size %i", bytes);
		}
		// if we run out of room, just start dropping commands
		return NULL;
	}

	cmdList->used += bytes;

	return cmdList->cmds + cmdList->used - bytes;
}

/*
=============
RE_SetColor

Passing NULL will set the color to white
=============
*/
void RE_SetColor(const float* rgba) {
	setColorCommand_t* cmd;
	cmd = static_cast<setColorCommand_t*>(R_GetCommandBuffer(sizeof(*cmd)));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_SET_COLOR;
	if (!rgba) {
		static float colorWhite[4] = { 1, 1, 1, 1 };

		rgba = colorWhite;
	}

	cmd->color[0] = rgba[0];
	cmd->color[1] = rgba[1];
	cmd->color[2] = rgba[2];
	cmd->color[3] = rgba[3];
}

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

/*SYS STUFF*/
int Sys_GetModulePathInfo(HMODULE module, char** path, char** filename, char** extension) {
	int sep = '/';
	static char szFileName[MAX_PATH + 1];
	if (path)
		*path = NULL;
	if (filename)
		*filename = NULL;
	if (extension)
		*extension = NULL;

	GetModuleFileNameA(module, szFileName, MAX_PATH);

	char* fn = strrchr(szFileName, sep);
	if (fn == nullptr) {
		sep = '\\';
		fn = strrchr(szFileName, sep);
	}
	if (fn != NULL) {
		*fn++ = 0;

		char* ext = strrchr(fn, '.');

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

bool Sys_ElevateProgram(char* arg3, bool restart) {
	if (Sys_IsAdmin() && !restart)
		return false; //we already are admin

	char* fn;

	Sys_GetModulePathInfo(NULL, NULL, &fn, NULL);

	char* arg;
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

#if 0
	static void __call(unsigned int off, unsigned int loc) {
#ifdef _WIN32
		DWORD tmp;
		VirtualProtect((void*)off, 5, PAGE_EXECUTE_READWRITE, &tmp);
#endif
		int foffset = loc - (off + 5);
		memcpy((void*)(off + 1), &foffset, 4);
		FlushInstructionCache(GetCurrentProcess(), (void*)off, 5);
#ifdef _WIN32
		VirtualProtect((void*)off, 5, tmp, &tmp);
#endif
	}
#endif
}