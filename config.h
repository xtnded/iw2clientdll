// depends on USERNAME=$(UserName) in Solution project
#define R 1
#define kung 2

#if USERNAME == R
	#define USE_CURL 1
#endif

#if USERNAME == kung
	#define USE_CURL 0
#endif
