#include "chakracore.h"
#include <stdio.h>
#include <Windows.h>

typedef int (*load_chakracore)();

int chakracore_init() {
	HMODULE handle = LoadLibrary("ChakraCoreInterface.dll");
	auto func_load_chakracore = (load_chakracore)GetProcAddress(handle, "load_chakracore");
	int a = (int)GetProcAddress(handle, "load_chakracore");

	printf("cci handle=%d func_load_chakracore=%d\n", handle, func_load_chakracore);

	if (func_load_chakracore)
		func_load_chakracore();

	return 1;
}
