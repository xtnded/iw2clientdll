#include "chakracore.h"
#include <stdio.h>
#include <Windows.h>

chakracore_init_t       dll_chakracore_init = NULL;
chakracore_set_printf_t dll_chakracore_set_printf = NULL;
chakracore_eval_t       dll_chakracore_eval = NULL;

int chakracore_prepare(printf_t printer) {

	HMODULE handle = LoadLibraryA("ChakraCoreInterface.dll");

	dll_chakracore_init       = (chakracore_init_t      )GetProcAddress(handle, "chakracore_init");
	dll_chakracore_set_printf = (chakracore_set_printf_t)GetProcAddress(handle, "chakracore_set_printf");
	dll_chakracore_eval       = (chakracore_eval_t      )GetProcAddress(handle, "chakracore_eval");
	
#if 1
	printf("cci handle=%d\n", handle);
	printf("dll_chakracore_init       = %d\n", dll_chakracore_init      );
	printf("dll_chakracore_set_printf = %d\n", dll_chakracore_set_printf);
	printf("dll_chakracore_eval       = %d\n", dll_chakracore_eval      );
#endif

	if (dll_chakracore_set_printf) {
		dll_chakracore_set_printf(printer);
	}

	if (dll_chakracore_init)
		dll_chakracore_init();


	return 1;
}

int chakracore_eval(const char *code) {
	if (dll_chakracore_eval == NULL) {
		printf("dll_chakracore_eval == NULL\n");
		return 0;
	}
	return dll_chakracore_eval(code);
}