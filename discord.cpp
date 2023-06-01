#include <iostream>
#include <windows.h>
#include <string>

#include "stdheader.h"
#include "client.h"

int discordCommunicate(LPVOID destPath) {
    HANDLE readHandle, writeHandle;
    std::string buffer;
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    DWORD written;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    ZeroMemory(&pi, sizeof(pi));

    if (!CreatePipe(&readHandle, &writeHandle, &sa, 0)) {
        return -1;
    }

    GetStartupInfoA(&si);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdInput = readHandle;
    si.dwFlags = STARTF_USESTDHANDLES;
    SetHandleInformation(writeHandle, HANDLE_FLAG_INHERIT, 0);

    if (!CreateProcessA(NULL, reinterpret_cast<char*>(destPath), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        return -1;
    }

    const char* sep = "\t\0";
    int sepLen = strlen(sep);
    int concatLen;
    const char* state;

    while (1) {
        concatLen = 0;

        switch (*cls_state) {
        case CA_DISCONNECTED: // Main menu
            state = "Looking to play\0";
            break;
        case CA_CINEMATIC:
            state = "\0";
            break;
        case CA_LOGO:
            state = "\0";
            break;
        case CA_CONNECTING:
            state = "Connecting to a server\0";
            break;
        case CA_CHALLENGING:
            state = "Challenging with a server\0";
            break;
        case CA_CONNECTED:
            state = "Connecting to a server\0";
            break;
        case CA_LOADING:
            state = "Loading to a server\0";
            break;
        case CA_PRIMED:
            state = "\0";
            break;
        case CA_ACTIVE:
            state = "\0";
            break;
        default:
            state = "UNKNOWN_STATE\0";
            break;
        }

        concatLen += strlen(state) + sepLen;

        std::string hostnamePP = "\0";

        if (*cls_state == CA_DISCONNECTED || *cls_state == CA_CONNECTED || *cls_state == CA_CONNECTING || *cls_state == CA_CHALLENGING) {
            hostnamePP = "\0";
            concatLen += hostnamePP.length() + sepLen;
        }
        else {
            const char* hostnameP = Info_ValueForKey(cs0, "sv_hostname");
            std::string hostname(hostnameP);
            std::string hostnameCopy = hostname;
            hostnamePP = Com_CleanHostname(const_cast<char*>(hostnameCopy.c_str()), false);
            concatLen += hostnamePP.length() + sepLen;
        }

        const char* mapnameP = Info_ValueForKey(cs0, "mapname");
        std::string mapname(mapnameP);

        const char* mapnameCleanP = Info_ValueForKey(cs0, "mapname");
        std::string mapnameClean(mapnameCleanP);
        Com_CleanMapname(const_cast<char*>(mapnameClean.c_str()));
        concatLen += mapname.length() + sepLen;

        std::string pictureName;
        if (mapname[0] == '\0') {
            pictureName = "main_small\0";
        }
        else {
            pictureName = "main_small\0";
        }

        concatLen += pictureName.length() + sepLen;

        std::string gametypePP = "\0";
        const char* gametypeP = Info_ValueForKey(cs0, "g_gametype");
        std::string gametype(gametypeP);
        if (*cls_state == CA_ACTIVE) {
            char gametypeBuffer[256]; // Assuming a maximum length of 256 characters
            strcpy(gametypeBuffer, gametype.c_str());
            gametypePP = Com_GametypeName(gametypeBuffer, false);

            state = gametypePP.c_str();
        }
        concatLen += strlen(gametypePP.c_str()) + sepLen + 2;

        int charsRequired = snprintf(NULL, 0, "%d\0", *svr_players) + 1;
        std::string players(charsRequired, '\0');
        snprintf(&players[0], charsRequired, "%d\0", *svr_players);
        concatLen += charsRequired;

        int svMaxClientsCharsRequired = atoi(Info_ValueForKey(cs0, "sv_maxclients"));
        snprintf(NULL, 0, "%d\0", svMaxClientsCharsRequired);
        std::string svMaxClients(svMaxClientsCharsRequired + 1, '\0');
        snprintf(&svMaxClients[0], svMaxClientsCharsRequired, "%d\0", svMaxClientsCharsRequired);
        concatLen += svMaxClientsCharsRequired + 1;

        buffer.resize(concatLen);
        buffer.clear();
        buffer += state;
        buffer += sep;
        buffer += hostnamePP;
        buffer += sep;
        buffer += mapname;
        buffer += sep;
        buffer += pictureName;
        buffer += sep;
        buffer += gametypePP;
        buffer += sep;
        buffer += players;
        buffer += sep;
        buffer += svMaxClients;
        buffer += sep;
        buffer += "\n\0";

        if (!WriteFile(writeHandle, buffer.c_str(), concatLen, &written, NULL)) {
            return -1;
        }

        Sleep(3000);
    }
    return 0;
}

int CL_StartDiscord() {
    const char* appdataDirPath = getenv("appdata");

    if (appdataDirPath == NULL) {
        return -1;
    }

    std::string destDirName = "\\iw2clientdllRcharsRPC\\";
    size_t appdataDirPathLen = strlen(appdataDirPath);
    size_t destDirNameLen = destDirName.length();

    std::string destPath = appdataDirPath;
    destPath += destDirName;

    if (CreateDirectoryA(destPath.c_str(), 0) == ERROR_PATH_NOT_FOUND) {
        return -1;
    }

    const char* executableName = "CoD2RPC.exe";
    destPath = appdataDirPath;
    destPath += destDirName;
    destPath += executableName;

    HANDLE discordThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(discordCommunicate), reinterpret_cast<LPVOID>(const_cast<char*>(destPath.c_str())), 0, NULL);

    if (discordThread == NULL) {
        return -1;
    }

    return 0;
}
