#include "../qcommon/qcommon.h"
#include "../util//memutil/memutil.h"

bool fix_bugs() {
	//MAX_PACKET_USERCMDS SPAM FIX
	PATCH_PUSH_STRING_PTR_VALUE(0x409004, "");

	//Error trying to lock sample! SPAM FIX
	PATCH_PUSH_STRING_PTR_VALUE(0x4BA55F, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BAF24, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BAF52, "");

	//DSERR_INVALIDPARAM SPAM FIX
	PATCH_PUSH_STRING_PTR_VALUE(0x4BAF36, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BA5F7, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BAF36, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BB06A, "");

	//Offset : %i, length: %i SPAM FIX
	PATCH_PUSH_STRING_PTR_VALUE(0x4BAF45, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BA535, "");

	//Error: Failed to get cursor positions SPAM FIX
	PATCH_PUSH_STRING_PTR_VALUE(0x4BA7EC, "");
	PATCH_PUSH_STRING_PTR_VALUE(0x4BAEA6, "");

	//nop splash screen
	__nop(0x466555, 0x466555 + 5);
	__nop(0x046664A, 0x046664A + 2);

	//fix for the blackscreen bug by php
	__nop(0x4B9569, 2);

	//patch mic crash issue cod2
	XUNLOCK((void*)0x005B375C, 1);
	XUNLOCK((void*)0x005B372D, 1);
	*(unsigned char*)0x005B375C = 'x';
	*(unsigned char*)0x005B372D = 'x';

	#ifdef DEBUG
		//G_ModelIndex MAX_MODELS increase attempt to 1024
		* (int*)0x50E37A = 0x400;
	#endif

	return true;
}