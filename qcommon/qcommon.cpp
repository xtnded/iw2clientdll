#include "qcommon.h"
#include "timeapi.h"
#include "../util/stdafx/stdafx.h"
#include "cmd.h"

Dvar_RegisterBool_t Dvar_RegisterBool = (Dvar_RegisterBool_t)0x438040;
Dvar_RegisterFloat_t Dvar_RegisterFloat = (Dvar_RegisterFloat_t)0x438100;
Dvar_RegisterString_t Dvar_RegisterString = (Dvar_RegisterString_t)0x437DE0;
//Dvar_RegisterInt_t Dvar_RegisterInt = (Dvar_RegisterInt_t)0x437CD0; //crashing
Dvar_SetFromStringByName_t Dvar_SetFromStringByName = (Dvar_SetFromStringByName_t)0x439150;
Dvar_Set_t Dvar_Set = (Dvar_Set_t)0x439E90;
Dvar_SetVariant_t Dvar_SetVariant = (Dvar_SetVariant_t)0x437090;
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
SCR_DrawSmallStringExt_t SCR_DrawSmallStringExt = (SCR_DrawSmallStringExt_t)0x4146A0;
CG_DrawBigDevStringColor_t CG_DrawBigDevStringColor = (CG_DrawBigDevStringColor_t)0; //0x4C2600 crashing reason is unknown
R_DrawText_t R_DrawText = (R_DrawText_t)GFX_OFF(0x1000C030);
//Info_ValueForKey_t Info_ValueForKey = (Info_ValueForKey_t)0x44AA90; //when i call it from the game address it doesn't work properly
FS_ReadFile_t FS_ReadFile = (FS_ReadFile_t)0x423240;
SV_XModelGet_t SV_XModelGet = (SV_XModelGet_t)0x490330;
SV_SetConfigstring_t SV_SetConfigstring = (SV_SetConfigstring_t)0x457CE0;
Scr_Error_t Scr_Error = (Scr_Error_t)0x44A990;
//Cmd_Argc_t Cmd_Argc = (Cmd_Argc_t)0xB1A480;
//Cmd_Argv_t Cmd_Argv = (Cmd_Argv_t)0xB1A498; //dword_B1A498
Dvar_GetInt_t Dvar_GetInt = (Dvar_GetInt_t)0x4373A0;
//Cbuf_ExecuteText_t Cbuf_ExecuteText = (Cbuf_ExecuteText_t)0x420B30; //cdecl
Cmd_ExecuteString_t Cmd_ExecuteString = (Cmd_ExecuteString_t)0x4214C0; //usercall
Cbuf_ExecuteInternal_t Cbuf_ExecuteInternal = (Cbuf_ExecuteInternal_t)0x420C20; 

const char* __cdecl va(const char* format, ...) {
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

	const char* arg;
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

void Q_strncpyz(char* dest, const char* src, int destsize) {
	if (!src) {
		Com_Error(ERR_FATAL, "Q_strncpyz: NULL src");
	}
	if (destsize < 1) {
		Com_Error(ERR_FATAL, "Q_strncpyz: destsize < 1");
	}

	strncpy(dest, src, destsize - 1);
	dest[destsize - 1] = 0;
}

int Q_stricmpn(const char* s1, const char* s2, int n) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;       // strings are equal until end point
		}

		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);

	return 0;       // strings are equal
}

int Q_stricmp(const char* s1, const char* s2) {
	return (s1 && s2) ? Q_stricmpn(s1, s2, 99999) : -1;
}

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
const char* Info_ValueForKey(const char* s, const char* key) {
	char pkey[BIG_INFO_KEY];
	static char value[2][BIG_INFO_VALUE];   // use two buffers so compares
	// work without stomping on each other
	static int valueindex = 0;
	char* o;

	if (!s || !key) {
		return "";
	}

	if (strlen(s) >= BIG_INFO_STRING) {
		Com_Error(ERR_DROP, "Info_ValueForKey: oversize infostring [%s] [%s]", s, key);
	}

	valueindex ^= 1;
	if (*s == '\\') {
		s++;
	}
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s) {
				return "";
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!Q_stricmp(key, pkey)) {
			return value[valueindex];
		}

		if (!*s) {
			break;
		}
		s++;
	}

	return "";
}

char* Q_CleanStr(char* string, bool colors) {
	char* d;
	char* s;
	int c;

	s = string;
	d = string;
	while ((c = *s) != 0) {
		if (Q_IsColorString(s) && !colors) {
			s++;
		}
		else if (c >= 0x20 && c <= 0x7E) {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

char* Com_CleanHostname(char* string, bool colors) {
	char hostname[MAX_HOSTNAME_LENGTH];
	Q_strncpyz(hostname, string, sizeof(hostname));

	// Remove symbols (and colors for RPC).
	Q_CleanStr(hostname, colors);

	// Check if hostname is empty when symbols are removed.
	if (hostname[0] == '\0') strncpy(hostname, "Unnamed Server", sizeof(hostname));

	// Remove leading spaces.
	int i = 0;
	while (isspace(hostname[0]) || hostname[0] == '!' || hostname[0] == '\'') {
		i = 0;
		while (hostname[i]) {
			hostname[i] = hostname[i + 1];
			i++;
		}
	}

	// Check if hostname is empty when leading spaces are removed.
	if (hostname[0] == '\0') strncpy(hostname, "Unnamed Server", sizeof(hostname));

	// Check if hostname is empty when colors are removed.
	if (colors) {
		char tempHostname[MAX_HOSTNAME_LENGTH];
		Q_strncpyz(tempHostname, hostname, sizeof(tempHostname));
		Q_CleanStr(tempHostname, false);
		if (tempHostname[0] == '\0') strncpy(hostname, "Unnamed Server", sizeof(hostname));
	}

	return hostname;
}

char* Com_CleanMapname(char* mapname) {
	for (int i = 0; mapname[i]; i++) // All to lowercase.
		mapname[i] = tolower(mapname[i]);

	if (strstr(mapname, "mp_") != NULL) // Remove mp_ and xp_ prefixes.
		mapname = mapname + 3;

	mapname[0] = toupper(mapname[0]); // First letter uppercase.

	for (int i = 0; mapname[i]; i++) { // Replace _ with space and uppercase next letter.
		if (mapname[i] == '_') {
			mapname[i] = ' ';
			mapname[i + 1] = toupper(mapname[i + 1]);
		}
	}

	return mapname;
}

const char* GetStockGametypeName(char* gt) {
	char s[64] = { 0 };
	Q_strncpyz(s, gt, sizeof(s));

	if (!strcmp(s, "dm"))
		return "Deathmatch";
	else if (!strcmp(s, "tdm"))
		return "Team Deathmatch";
	else if (!strcmp(s, "re"))
		return "Retrieval";
	else if (!strcmp(s, "bel"))
		return "Behind Enemy Lines";
	else if (!strcmp(s, "sd"))
		return "Search & Destroy";

	return NULL;
}

char* GetTxtGametypeName(char* gt, bool colors) {
	char* name;
	const char* file = va("maps/mp/gametypes/%s.txt", gt);
	FS_ReadFile(file, (void**)&name);

	if (!name) return NULL;

	// Reimplementation of Q_CleanStr (remove quotes too).
	char* d = name, * s = name;
	int c;
	while ((c = *s) != 0) {
		if (Q_IsColorString(s) && !colors) s++;
		else if (c >= 0x20 && c <= 0x7E && c != 0x22) *d++ = c;
		s++;
	}
	*d = '\0';

	return name;
}
const char* Com_GametypeName(char* gt, bool colors) {
	if (!gt || !*gt) return "Unknown Gametype";

	const char* name = GetStockGametypeName(gt);
	if (!name) name = GetTxtGametypeName(gt, colors);

	if (name)
		return name;
	else
		return (colors) ? gt : Q_CleanStr(gt, colors);
}

/*
============
Cbuf_InsertText
Adds command text immediately after the current command
Adds a \n to the text
============
*/
cmd_t		cmd_text;
void Cbuf_InsertText(const char* text)
{
	int len;
	int i;

	len = strlen(text) + 1;

	if (len + cmd_text.cursize > cmd_text.maxsize)
	{
		Com_Printf("Cbuf_InsertText overflowed\n");
		return;
	}

	// move the existing command text
	for (i = cmd_text.cursize - 1; i >= 0; i--)
	{
		cmd_text.data[i + len] = cmd_text.data[i];
	}

	// copy the new text in
	memcpy(cmd_text.data, text, len - 1);

	// add a \n
	cmd_text.data[len - 1] = '\n';
	cmd_text.cursize += len;
}

/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText(int exec_when, const char* text)
{
	switch (exec_when)
	{
	case EXEC_NOW:
			Cbuf_ExecuteInternal();
		break;
	case EXEC_INSERT:
		Cbuf_InsertText(text);
		break;
	case EXEC_APPEND:
		Cbuf_AddText(text);
		break;
	default:
		Com_Error(ERR_FATAL, "Cbuf_ExecuteText: bad exec_when");
	}
}

int Com_sprintf(char* dest, size_t size, const char* format, ...)
{
	int result;
	va_list va;

	va_start(va, format);
	result = vsnprintf(dest, size, format, va);
	va_end(va);

	dest[size - 1] = '\0';

	return result;
}

extern dvar_t* com_developer;
/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void Com_DPrintf(conChannel_t channel, const char* fmt, ...) 
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!com_developer || !com_developer->current.integer) {
		return;			// don't confuse non-developers with techie stuff...
	}

	msg[0] = '^';
	msg[1] = '2';

	va_start(argptr, fmt);
	vsnprintf(&msg[2], (sizeof(msg) - 3), fmt, argptr);
	va_end(argptr);

	Com_PrintMessage(channel, msg);
}

void Z_Free(void* ptr) {
	free(ptr);
}

void* GetClearedMemory(int size) {
	void* ptr;
	ptr = (void*)malloc(size);
}

void* Z_Malloc(int size) {
	return GetClearedMemory(size);
}

int Q_CountChar(const char* string, char tocount)
{
	int count;

	for (count = 0; *string; string++)
	{
		if (*string == tocount)
			count++;
	}

	return count;
}

short   ShortSwap(short l)
{
	byte    b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

int    LongSwap(int l)
{
	byte    b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

float FloatSwap(float f) {
	union
	{
		float f;
		byte b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}