#include <ShlObj.h> //header for VirtualProtect,DWORD etc

#include "memutil.h"

#ifdef _WIN32
#define JUNK_CODE_ONE        \
    __asm{push eax}            \
    __asm{xor eax, eax}        \
    __asm{setpo al}            \
    __asm{push edx}            \
    __asm{xor edx, eax}        \
    __asm{sal edx, 2}        \
    __asm{xchg eax, edx}    \
    __asm{pop edx}            \
    __asm{or eax, ecx}        \
    __asm{pop eax}
#else
#define JUNK_CODE_ONE
#endif

void XUNLOCK(void* addr, size_t len) {
	if (_WIN32) {
		DWORD tmp;
		VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &tmp);
	}
}

void _memset(void* addr, int v, size_t len) {
	if (_WIN32) {
		DWORD tmp;
		VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &tmp);
	}
	memset(addr, v, len);
	FlushInstructionCache(GetCurrentProcess(), (void*)addr, len);
	//#ifdef _WIN32
	//	//VirtualProtect((void*)addr, len, tmp, &tmp);
	//#endif
}

void _memcpy(void* v1, void* v2, size_t len) {
	if (_WIN32) {
		DWORD tmp;
	}
	memcpy(v1, v2, len);
	FlushInstructionCache(GetCurrentProcess(), (void*)v1, len);
	//#ifdef _WIN32
	//	//VirtualProtect((void*)v1, len, tmp, &tmp);
	//#endif
}

int search_memory(int current, int end, BYTE* bytes, size_t len) {
	while ((memcmp((void*)current, bytes, len)) && (current < end))
		current++;
	return (current == end) ? -1 : current;
}

int patch_memory(int start, int end, BYTE* search_for, BYTE* patched, size_t len) {
	int s = search_memory(start, end, search_for, len);
#ifdef _WIN32
	DWORD tmp;
#endif
	if (-1 != s) {
		if (_WIN32) {
			VirtualProtect((void*)s, len, PAGE_EXECUTE_READWRITE, &tmp);
		}
		memcpy((void*)s, patched, len);
		FlushInstructionCache(GetCurrentProcess(), (void*)s, len);
		if (_WIN32) {
			VirtualProtect((void*)s, len, tmp, &tmp);
		}
	}
	return s;
}

/*
Hooking
*/

void __nop(unsigned int start, unsigned int end) {
	int len = (end < start) ? end : (end - start);
#ifdef _WIN32
		DWORD tmp;
		VirtualProtect((void*)start, len, PAGE_EXECUTE_READWRITE, &tmp);
#endif
	memset((void*)start, 0x90, len);
	FlushInstructionCache(GetCurrentProcess(), (void*)start, len);
	if (_WIN32) {
		VirtualProtect((void*)start, len, tmp, &tmp);
	}
}

void __jmp(unsigned int off, unsigned int loc) {
#ifdef _WIN32
	DWORD tmp;
	VirtualProtect((void*)off, 5, PAGE_EXECUTE_READWRITE, &tmp);
#endif
	* (unsigned char*)off = 0xe9;
	int foffset = loc - (off + 5);
	memcpy((void*)(off + 1), &foffset, 4);
	FlushInstructionCache(GetCurrentProcess(), (void*)off, 5);
	if (_WIN32) {
		VirtualProtect((void*)off, 5, tmp, &tmp);
	}
}

void __call(unsigned int off, unsigned int loc) {
#ifdef _WIN32
	DWORD tmp;
	VirtualProtect((void*)off, 5, PAGE_EXECUTE_READWRITE, &tmp);
#endif
	int foffset = loc - (off + 5);
	memcpy((void*)(off + 1), &foffset, 4);
	FlushInstructionCache(GetCurrentProcess(), (void*)off, 5);
	if (_WIN32) {
		VirtualProtect((void*)off, 5, tmp, &tmp);
	}
}

#if 0//TODO?
static void dumpbase(int* base, size_t len, const char* fn) {
	static char dumpbase_path[64];

	_snprintf(dumpbase_path, 63, "./dumpfiles/%s.dump", fn);

	unsigned int result;
	float rf;
	char* rstr;

	FILE* f = fopen(dumpbase_path, "wb");
	if (f) {
		fprintf(f, "File: %s\nDUMPFILE\n\n");

		for (unsigned int i = 0; i < len; i++) {
			result = *(unsigned int*)(base + i);
			rf = *(float*)(base + i);
			rstr = (char*)(base + i);
			fprintf(f, "%d: Hex: %x, Decimal: %d, Float: %f, String: %s\n", i, result, result, rf, rstr);
		}
		fclose(f);
	}

	//Com_Printf("Successfully dumped '%s' to location '%s'.\n", fn, dumpbase_path);
}
#endif