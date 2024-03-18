#pragma once

#include "../util/stdafx/stdafx.h"
#include "../util/memutil/memutil.h"

#define cls_realtime ((int*)0x68A520)
#define cls_state ((int*)0x609FE0)
#define svr_players ((int*)0x1518F80)
//#define cls_downloadRestart ((PINT)0x15EEFBC)
//#define clc_downloadList ((PCHAR)0x15EEBBC)
#define clc_stringData ((PCHAR)0x96FD5C)
#define clc_stringOffsets ((PINT)0x96DD5C)
//#define cls_downloadBlock ((PINT)0x15EEBB0)
//#define cls_downloadCount ((PINT)0x15EEBB4)
//#define cls_downloadName ((PCHAR)0x15EEAAC)
//#define cls_downloadTempName ((PCHAR)0x15EE9AC)
//#define clc_serverAddress ((netadr_t*)0x15CE86C)
//#define clc_connectTime ((PINT)0x15CE880)
//#define clc_demoplaying ((PINT)0x15EF004)
//#define clc_connectPacketCount ((PINT)0x15CE884)
//#define cls_keyCatchers ((PINT)0x155F2C4)
//#define cls_keyCatchers ((PINT)0x96B654)
//#define cls_servername ((char*)0x155F2CC)

#define cs0 (clc_stringData + clc_stringOffsets[0])
#define cs1 (clc_stringData + clc_stringOffsets[1])

//static void(*CL_AddReliableCommand)(const char *cmd) = (void(*)(const char*))0x40E2F0;
//static const char* (*CL_TranslateStringBuf)(const char *string) = (const char*(*)(const char*))0x4A9E20;

static bool unlock_client_structure() {
	__try {

		XUNLOCK((void*)cls_realtime, sizeof(int));
		XUNLOCK((void*)cls_state, sizeof(int));
		//XUNLOCK((void*)cls_downloadRestart, 4);
		XUNLOCK((void*)0x609FE0, 4);
		//XUNLOCK((void*)clc_connectTime, 4);
		//XUNLOCK((void*)clc_demoplaying, 4);
		//XUNLOCK((void*)clc_downloadList, 4096);
		//XUNLOCK((void*)cls_downloadBlock, 4);
		//XUNLOCK((void*)cls_downloadCount, 4);
		//XUNLOCK((void*)0x, 64);
		//XUNLOCK((void*)cls_downloadTempName, 64);
	}
	__except (1) {
		return false;
	}
	return true;
}