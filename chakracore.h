#pragma once



typedef int(*printf_t)(const char *format, ...);
typedef int(*chakracore_init_t)();
typedef int(*chakracore_set_printf_t)(printf_t);
typedef int(*chakracore_eval_t)(const char *code);

int chakracore_prepare(printf_t printer);
int chakracore_eval(const char *code);