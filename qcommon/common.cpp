#include "qcommon.h"
#include "common.h"

dvar_t* ui_playerProfileAlreadyChosen;
dvar_t* com_playerProfile;

Com_BuildPlayerProfilePath_t Com_BuildPlayerProfilePath = (Com_BuildPlayerProfilePath_t)0x42D430;
Com_Parse_t Com_Parse = (Com_Parse_t)0x42D260;

void Com_SetPlayerProfile(char* profilename) {
    dvar_t* nameVariant;
    char configPath[88];

    // Set player profile
    Dvar_SetString(com_playerProfile, profilename);

    // Build player profile path
    Com_BuildPlayerProfilePath(configPath, 0x40, "config_mp.cfg");

    // Execute startup configs
    Com_ExecStartupConfigs(configPath);

    // Get name variant
    nameVariant = Dvar_GetVariantString("name");

    // Set name if not already set
    if (!nameVariant || !nameVariant->current.string)
        Dvar_SetStringByName("name", profilename);
}

void Com_ExecStartupConfigs(const char* configFile)
{
    Cbuf_AddText("exec default_mp.cfg\n");
    Cbuf_AddText("exec language.cfg\n");
    if (configFile)
        Cbuf_AddText(va("exec %s\n", configFile));
    Cbuf_Execute();
    *(int*)0x42D326 = 1;
    Cbuf_Execute();
    *(int*)0x42D326 = 0;
    if (Com_SafeMode())
        Cbuf_AddText("exec safemode_mp_server.cfg\n");
    Cbuf_Execute();
}

void Com_InitPlayerProfiles(void) {
    void(*oCom_ExecStartupConfigs)(void) = (void(*)(void))0x42D260;
    oCom_ExecStartupConfigs();
    char* activeProfileName;
    char* fileData;
    char** fileListHandle;
    int profileCount;
    int profileExists;
    char profileNameBuffer[64];
    union DvarValue trueValue;
    trueValue.enabled = true;

    // Read active profile data
    if ((FS_ReadFile("players/active.txt", (void **)&fileData) & 0x80000000) != 0)
        return Com_ExecStartupConfigs(0);
    activeProfileName = Com_Parse((char *)fileData[0]);
    FS_FreeFile((void**)fileData[0]);
    if (!*activeProfileName)
        return Com_ExecStartupConfigs(0);

    // Check if active profile exists
    fileListHandle = FS_ListFiles("players", "/", &profileCount);
    profileExists = 0;
    for (int i = 0; i < profileCount; i++) {
        if (strcmp(*(char**)(fileListHandle + 4 * i), activeProfileName) == 0) {
            profileExists = 1;
            break;
        }
    }
    FS_FreeFileList(fileListHandle);

    // If active profile doesn't exist, return
    if (!profileExists)
        return Com_ExecStartupConfigs(0);

    // Set player profile and execute startup configs
    Dvar_SetString(com_playerProfile, activeProfileName);
    Com_BuildPlayerProfilePath(profileNameBuffer, 0x40, "config_mp.cfg");
    return Com_SetPlayerProfile(activeProfileName);
}

void Com_Init_Try_Block_Function(char* commandLine)
{
    Com_InitPlayerProfiles();
}