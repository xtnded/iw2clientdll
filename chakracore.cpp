#include "chakracore.h"
#include <stdio.h>
#include <Windows.h>


typedef int (*printf_t)(const char *format, ...);
typedef int (*chakracore_init_t)();
typedef int (*chakracore_set_printf_t)(printf_t);

chakracore_init_t chakracore_init = NULL;
chakracore_set_printf_t chakracore_set_printf = NULL;

int chakracore_prepare() {

	HMODULE handle = LoadLibraryA("ChakraCoreInterface.dll");

	chakracore_init       = (chakracore_init_t      )GetProcAddress(handle, "chakracore_init");
	chakracore_set_printf = (chakracore_set_printf_t)GetProcAddress(handle, "chakracore_set_printf");
	
	printf("cci handle=%d chakracore_init=%d chakracore_set_printf=%d\n", handle, chakracore_init, chakracore_set_printf);

	if (chakracore_init)
		chakracore_init();

	if (chakracore_set_printf) {
		chakracore_set_printf(printf);
	}

	return 1;
}
