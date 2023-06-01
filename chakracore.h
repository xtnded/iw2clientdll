#pragma once

typedef int(__cdecl *printstring_t)(const char *format);
typedef int(*chakracore_init_t)();
typedef int(*chakracore_set_printstring_t)(printstring_t);
typedef int(*chakracore_eval_t)(const char *code);

int chakracore_prepare(printstring_t printer);
int chakracore_eval(const char *code);