#pragma once

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Shell32.lib")

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdarg>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <ShellAPI.h>

/*Render stuff*/
#define	MAX_RENDER_COMMANDS	0x40000
#define	SMP_FRAMES		2
#define	MAX_DRAWSURFS	0x10000
#define	MAX_DLIGHTS		32			// can't be increased, because bit flags are used on surfaces
#define	MAX_ENTITIES	1023		// can't be increased without changing drawsurf bit packing

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

extern DWORD gfx_dll_mp;
#define GFX_OFF(x) (gfx_dll_mp + (x - 0x10000000))

#define MOD_NAME "Call of Duty 2x"
#define MsgBox(x) MessageBoxA(0,x,0,0)

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

#define	CVAR_ARCHIVE		1
#define	CVAR_USERINFO		2
#define	CVAR_SERVERINFO		4
#define	CVAR_SYSTEMINFO		8
#define	CVAR_INIT			16
#define	CVAR_LATCH			32
#define	CVAR_ROM			64
#define CVAR_CHEAT			128
#define	CVAR_TEMP			256
#define CVAR_NORESTART		1024
#define	CVAR_USER_CREATED	16384

/*INFO STRINGS*/
#define BIG_INFO_STRING     8192    // used for system info key only
#define BIG_INFO_KEY        8192
#define BIG_INFO_VALUE      8192

#define MAX_HOSTNAME_LENGTH 1024

#define Q_COLOR_ESCAPE  '^'
#define Q_IsColorString( p )  ( p && *( p ) == Q_COLOR_ESCAPE && *( ( p ) + 1 ) && *( ( p ) + 1 ) != Q_COLOR_ESCAPE )

#define PATCH_PUSH_STRING_PTR_VALUE(offset, new_str) \
	XUNLOCK((void*)offset, 10); \
	*(const char **)(offset + 1) = new_str;

#define PATCH_PUSH_INT_PTR_VALUE(offset, new_int) \
	XUNLOCK((void*)offset, 10); \
	*(int *)(offset + 1) = new_int;

static float vColorBlack[4] = { 0,0,0,1 };
static float vColorWhite[4] = { 1, 1, 1, 1 };
static float vColorSelected[4] = { 1, 1, 0, 1 };

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
#pragma pack(push, 4)
struct vec4_t
{
	float x, y, z, w;
};
#pragma pack(pop)
typedef vec_t vec5_t[5];

typedef struct {
	byte	cmds[MAX_RENDER_COMMANDS];
	int		used;
} renderCommandList_t;

/*Dvars*/
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
		const char** strings;
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

union DvarValue
{
	bool enabled;
	int integer;
	float value;
	vec4_t vector;
	const char *string;
	char color[4];
};

typedef struct dvar_s {
	int name;
	short flags;
	char type;
	char modified;
	DvarValue current;
	int latched;
	int reset;
	DvarLimits domain;
	int next;
	int hashNext;
} dvar_t;

typedef struct
{
	byte red;
	byte green;
	byte blue;
	byte alpha;
}ucolor_t;

/*Cvars*/
typedef enum {
	CVAR_BOOL,
	CVAR_FLOAT,
	CVAR_VEC2,
	CVAR_VEC3,
	CVAR_VEC4,
	CVAR_INT,
	CVAR_ENUM,
	CVAR_STRING,
	CVAR_COLOR
}cvarType_t;

typedef union
{
	float floatval;
	int integer;
	const char* string;
	byte boolean;
	vec4_t vec4;
	vec3_t vec3;
	vec2_t vec2;
	ucolor_t color;
}CvarValue_t;


typedef struct {
	union {
		int imin;
		float fmin;
		int enumCount;
	};
	union {
		int imax;
		float fmax;
		const char** enumStrings;
	};
}CvarLimits_t;

typedef struct cvar_s
{
	char* name;
	unsigned short flags;
	byte type;
	byte modified;
	union
	{
		float floatval;
		int integer;
		char* string;
		byte boolean;
		vec2_t vec2;
		vec3_t vec3;
		vec4_t vec4;
		ucolor_t color;
	};
	union
	{
		float latchedFloatval;
		int latchedInteger;
		char* latchedString;
		byte latchedBoolean;
		vec2_t latchedVec2;
		vec3_t latchedVec3;
		vec4_t latchedVec4;
		ucolor_t latchedColor;
	};
	union
	{
		float resetFloatval;
		int resetInteger;
		char* resetString;
		byte resetBoolean;
		vec2_t resetVec2;
		vec3_t resetVec3;
		vec4_t resetVec4;
		ucolor_t resetColor;
	};
	union
	{
		int imin;
		float fmin;
	};
	union
	{
		int imax;
		float fmax;
		const char** enumStr;
	};
	struct cvar_s* next;
	struct cvar_s* hashNext;
} cvar_t;

typedef enum
{
	NA_BOT, NA_BAD, NA_LOOPBACK, NA_BROADCAST, NA_IP
} netadrtype_t;

typedef struct
{
	netadrtype_t type;
	char ip[4];
	unsigned short port;
} netadr_t;

typedef enum {
	SF_BAD,
	SF_SKIP,				// ignore
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,
	SF_POLY,
	SF_MD3,
	SF_MD4,
	SF_FLARE,
	SF_ENTITY,				// beams, rails, lightning, etc that can be determined by entity
	SF_DISPLAY_LIST,

	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0x7fffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct dlight_s {
	vec3_t	origin;
	vec3_t	color;				// range from 0.0 to 1.0, should be color normalized
	float	radius;

	vec3_t	transformed;		// origin in local coordinate system
	int		additive;			// texture detail is lost tho when the lightmap is dark
} dlight_t;

typedef enum {
	RT_MODEL,
	RT_POLY,
	RT_SPRITE,
	RT_BEAM,
	RT_RAIL_CORE,
	RT_RAIL_RINGS,
	RT_LIGHTNING,
	RT_PORTALSURFACE,		// doesn't draw anything, just info for portals

	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

typedef struct {
	refEntityType_t	reType;
	int			renderfx;

	handle_t	hModel;				// opaque type outside refresh

	// most recent data
	vec3_t		lightingOrigin;		// so multi-part models can be lit identically (RF_LIGHTING_ORIGIN)
	float		shadowPlane;		// projection shadows go here, stencils go slightly lower

	vec3_t		axis[3];			// rotation vectors
	boolean	nonNormalizedAxes;	// axis are not normalized, i.e. they have scale
	float		origin[3];			// also used as MODEL_BEAM's "from"
	int			frame;				// also used as MODEL_BEAM's diameter

	// previous data for frame interpolation
	float		oldorigin[3];		// also used as MODEL_BEAM's "to"
	int			oldframe;
	float		backlerp;			// 0.0 = current, 1.0 = old

	// texturing
	int			skinNum;			// inline skin index
	handle_t	customSkin;			// NULL for default skin
	handle_t	customShader;		// use one image for the entire thing

	// misc
	byte		shaderRGBA[4];		// colors used by rgbgen entity shaders
	float		shaderTexCoord[2];	// texture coordinates used by tcMod entity modifiers
	float		shaderTime;			// subtracted from refdef time to control effect start times

	// extra sprite information
	float		radius;
	float		rotation;
} refEntity_t;
// a trRefEntity_t has all the information passed in by
// the client game, as well as some locally derived info

typedef struct {
	refEntity_t	e;

	float		axisLength;		// compensate for non-normalized axis

	boolean	needDlights;	// true for bmodels that touch a dlight
	boolean	lightingCalculated;
	vec3_t		lightDir;		// normalized direction towards light
	vec3_t		ambientLight;	// color normalized to 0-255
	int			ambientLightInt;	// 32 bit rgba packed
	vec3_t		directedLight;
} trRefEntity_t;

typedef struct drawSurf_s {
	unsigned			sort;			// bit combination for fast compares
	surfaceType_t* surface;		// any of surface*_t
} drawSurf_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	byte		modulate[4];
} polyVert_t;

typedef struct srfPoly_s {
	surfaceType_t	surfaceType;
	handle_t		hShader;
	int				fogIndex;
	int				numVerts;
	polyVert_t* verts;
} srfPoly_t;

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;

typedef enum {
	RC_END_OF_LIST,
	RC_SET_COLOR,
	RC_STRETCH_PIC,
	RC_DRAW_SURFS,
	RC_DRAW_BUFFER,
	RC_SWAP_BUFFERS,
	RC_SCREENSHOT
} renderCommand_t;

typedef struct {
	int		commandId;
	float	color[4];
} setColorCommand_t;

typedef struct
{
	uint64_t _bf0;
}GfxDrawSurfFields;

union GfxDrawSurf
{
	GfxDrawSurfFields fields;
	uint64_t packed;
};

#pragma pack(push, 8)
typedef struct MaterialInfo
{
	const char* name;
	char gameFlags;
	char sortKey;
	char textureAtlasRowCount;
	char textureAtlasColumnCount;
	union GfxDrawSurf drawSurf;
	int surfaceTypeBits;
}MaterialInfo_t;

struct Material
{
	MaterialInfo info;
	int64_t stateBits;
	short textureCount;
	short constantCount;
	int techniqueSet;
	int textures;
	int constants;
};

/* 7043 */
#pragma pack(push, 2)
typedef struct
{
	uint16_t letter;
	char x0;
	char y0;
	char dx;
	char pixelWidth;
	char pixelHeight;
	char pad;
	float s0;
	float t0;
	float s1;
	float t1;
}Glyph;
#pragma pack(pop)

/* 7044 */
typedef struct Font_s
{
	const char* fontName;
	int pixelHeight;
	int glyphCount;
	Material* material;
	Material* glowMaterial;
	Glyph* glyphs;
}Font_t;

typedef struct
{
	byte* data;
	int		maxsize;
	int		cursize;
} cmd_t;

enum conChannel_t
{
	CON_CHANNEL_DONT_FILTER = 0x0,
	CON_CHANNEL_ERROR = 0x1,
	CON_CHANNEL_GAMENOTIFY = 0x2,
	CON_CHANNEL_BOLDGAME = 0x3,
	CON_CHANNEL_LOGFILEONLY = 0x4,
};

extern const char* __cdecl va(const char* format, ...);
extern void Dvar_SetString(const char* _dvar, const char* strval);
extern void* R_GetCommandBuffer(int bytes);
extern void RE_SetColor(const float* rgba);
extern int Sys_IsAdmin();
extern int Sys_GetModulePathInfo(HMODULE module, char** path, char** filename, char** extension);
extern bool Sys_ElevateProgram(char* arg3, bool restart);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern const char* Info_ValueForKey(const char* s, const char* key);
extern char* Q_CleanStr(char* string, bool colors);
extern char* Com_CleanHostname(char* string, bool colors);
extern char* Com_CleanMapname(char* mapname);
extern const char* GetStockGametypeName(char* gt);
extern char* GetTxtGametypeName(char* gt, bool colors);
extern const char* Com_GametypeName(char* gt, bool colors);
extern void Cbuf_ExecuteText(int exec_when, const char* text);
extern int Com_sprintf(char* dest, size_t size, const char* format, ...);
extern void Com_DPrintf(conChannel_t channel, const char* fmt, ...);

typedef struct
{
	float scaleVirtualToReal[2];
	float scaleVirtualToFull[2];
	float scaleRealToVirtual[2];
	float virtualViewableMin[2];
	float virtualViewableMax[2];
	float realViewportSize[2];
	float realViewableMin[2];
	float realViewableMax[2];
	float subScreenLeft;
}ScreenPlacement;

typedef enum
{
	CA_DISCONNECTED = 0,
	CA_CINEMATIC = 1,
	CA_LOGO = 2,
	CA_CONNECTING = 3,
	CA_CHALLENGING = 4,
	CA_CONNECTED = 5,
	CA_LOADING = 6,
	CA_PRIMED = 7,
	CA_ACTIVE = 8
} connstate_t;

enum DvarSetSource
{
	DVAR_SOURCE_INTERNAL = 0x0,
	DVAR_SOURCE_EXTERNAL = 0x1,
	DVAR_SOURCE_SCRIPT = 0x2,
	DVAR_SOURCE_DEVGUI = 0x3,
};

typedef dvar_t *(*Dvar_RegisterBool_t)(const char* var_name, bool var_value, unsigned short flags);
extern Dvar_RegisterBool_t Dvar_RegisterBool;
typedef dvar_t *(*Dvar_RegisterFloat_t)(const char* var_name, float var_value, float var_min, float var_max, unsigned short flags);
extern Dvar_RegisterFloat_t Dvar_RegisterFloat;
typedef dvar_t* (*Dvar_RegisterString_t)(char const* dvarName, char const* value, unsigned short flags);
extern Dvar_RegisterString_t Dvar_RegisterString;
typedef dvar_t* (*Dvar_RegisterInt_t)(const char* dvarName, int value, int min, int max, unsigned short flags);
extern Dvar_RegisterInt_t Dvar_RegisterInt;
typedef dvar_t *(*Dvar_SetFromStringByName_t)(const char*, const char*);
extern Dvar_SetFromStringByName_t Dvar_SetFromStringByName;
typedef dvar_t *(*Dvar_Set_t)(const char*, const char*);
extern Dvar_Set_t Dvar_Set;
typedef dvar_t *(*Dvar_SetVariant_t)(const char* var_name, const char* value, bool force);
extern Dvar_SetVariant_t Dvar_SetVariant;
typedef dvar_t *(*Dvar_GetVariantString_t)(const char*);
extern Dvar_GetVariantString_t Dvar_GetVariantString;
typedef void(*Cmd_AddCommand_t)(const char*, void*);
extern Cmd_AddCommand_t Cmd_AddCommand;
typedef void(*Com_PrintMessage_t)(int, const char*);
extern Com_PrintMessage_t Com_PrintMessage;
typedef void(*CL_DrawString_t)(int a1, int a2, const char* a3, int a4, int a5);
extern CL_DrawString_t CL_DrawString;
typedef void(*Com_Error_t)(int a1, const char* Format, ...);
extern Com_Error_t Com_Error;
typedef void(*Cbuf_AddText_t)(const char*);
extern Cbuf_AddText_t Cbuf_AddText;
typedef void(*CL_DrawText_t)(char const* text, int maxChars, int font, float x, float y, int horzAlign, int vertAlign, float xScale, float yScale, float const* const color, int style);
extern CL_DrawText_t CL_DrawText;
typedef void(*SV_SendServerCommand_t)(int, const char*, ...);
extern SV_SendServerCommand_t SV_SendServerCommand;
typedef void(*Com_Quit_f_t)();
extern Com_Quit_f_t Com_Quit_f;
typedef int(*Com_Printf_t)(const char* format, ...);
extern Com_Printf_t Com_Printf;
typedef void(*SCR_DrawSmallStringExt_t)(signed int x, signed int y, const char* string, const float* setColor);//__cdecl
extern SCR_DrawSmallStringExt_t SCR_DrawSmallStringExt;
typedef void(*CG_DrawBigDevStringColor_t)(const char*, int, int, int, int, int);
extern CG_DrawBigDevStringColor_t CG_DrawBigDevStringColor;
typedef void(*R_DrawText_t)(float x, float y, int font, float scale, const float* color, const char* text, float spacing, int limit, int flags);
extern R_DrawText_t R_DrawText;
//typedef char* (*Info_ValueForKey_t)(const char * s, const char *key);
//extern Info_ValueForKey_t Info_ValueForKey;
typedef int(*FS_ReadFile_t)(const char* qpath, void** buffer);
extern FS_ReadFile_t FS_ReadFile;
typedef int(*SV_XModelGet_t)(const char* name);
extern SV_XModelGet_t SV_XModelGet;
typedef int(*SV_XModelGet_t)(const char* name);
extern SV_XModelGet_t SV_XModelGet;
typedef int(*SV_SetConfigstring_t)(unsigned int index, const char* val);
extern SV_SetConfigstring_t SV_SetConfigstring;
typedef int(*Scr_Error_t)(const char* error);
extern Scr_Error_t Scr_Error;
//typedef int(*Cmd_Argc_t)(void);
//extern Cmd_Argc_t Cmd_Argc;
//typedef const char* (*Cmd_Argv_t)(int arg);
//extern Cmd_Argv_t Cmd_Argv;
typedef int(*Dvar_GetInt_t)(const char* dvarName);
extern Dvar_GetInt_t Dvar_GetInt;
typedef void (*Cmd_ExecuteString_t)(const char* text);
extern Cmd_ExecuteString_t Cmd_ExecuteString;
typedef void (*Cbuf_ExecuteInternal_t)(void);
extern Cbuf_ExecuteInternal_t Cbuf_ExecuteInternal;

template <typename T, typename ... Ts>
T call(size_t addr, Ts ... ts) {
	T(*f)(...);
	*(T*)&f = (T)addr;
	return f(ts...);
}