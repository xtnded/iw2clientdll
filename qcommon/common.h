typedef void (*Com_BuildPlayerProfilePath_t)( char*, int, const char*, ...);
extern Com_BuildPlayerProfilePath_t Com_BuildPlayerProfilePath;

typedef char* (*Com_Parse_t)(char* data_p);
extern Com_Parse_t Com_Parse;

extern void Com_Init_Try_Block_Function(char* commandLine);
extern void Com_ExecStartupConfigs(const char* configFile);