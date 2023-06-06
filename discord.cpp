#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  // For malloc and free
#include <string.h>

#include "stdheader.h"
#include "discord.h"
#include "client.h"

DWORD WINAPI discord_communicate(LPVOID dest_path) {
    HANDLE read_handle, write_handle;
    char* buffer = NULL;
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    DWORD written;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    ZeroMemory(&pi, sizeof(pi));

    if (!CreatePipe(&read_handle, &write_handle, &sa, 0)) {
        return -1;
    }

    GetStartupInfoA(&si);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdInput = read_handle;
    si.dwFlags = STARTF_USESTDHANDLES;
    SetHandleInformation(write_handle, HANDLE_FLAG_INHERIT, 0);

    if (!CreateProcessA(NULL, (char*)dest_path, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
    	return -1;
    }

    const char* sep = "\t\0";
    int sep_len = strlen(sep);
    int concat_len;
    const char* state;

    while (1) {
        concat_len = 0;

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

        concat_len += strlen(state) + sep_len;

        char* hostname_pp = (char*)"\0";

        if (*cls_state == CA_DISCONNECTED || *cls_state == CA_CONNECTED || *cls_state == CA_CONNECTING || *cls_state == CA_CHALLENGING) {
            hostname_pp = (char*)"\0";
            concat_len += strlen(hostname_pp) + sep_len;
        }
        else {
            const char* hostname_p = Info_ValueForKey(cs0, "sv_hostname");
            char hostname[64] = { 0 };
            Q_strncpyz(hostname, hostname_p, sizeof(hostname));
            hostname_pp = Com_CleanHostname(hostname, false);
            concat_len += strlen(hostname_pp) + sep_len;
        }

        const char* mapname_p = Info_ValueForKey(cs0, "mapname");
        char mapname[64] = { 0 };
        Q_strncpyz(mapname, mapname_p, sizeof(mapname));

        const char* mapnameClean_p = Info_ValueForKey(cs0, "mapname");
        char mapnameClean[64] = { 0 };
        Q_strncpyz(mapnameClean, mapnameClean_p, sizeof(mapnameClean));
        Com_CleanMapname(mapnameClean);
        concat_len += strlen(mapname) + sep_len;

        char* picture_name;
        if (mapname[0] == '\0') {
            picture_name = (char*)"main_small\0";
        }
        else {
            picture_name = (char*)"main_small\0";
        }

        concat_len += strlen(picture_name) + sep_len;

        const char* gametype_pp = (char*)"\0";
        const char* gametype_p = Info_ValueForKey(cs0, "g_gametype");
        char gametype[64] = { 0 };
        Q_strncpyz(gametype, gametype_p, sizeof(gametype));
        if (*cls_state == CA_ACTIVE) {
            gametype_pp = Com_GametypeName(gametype, false);
            state = gametype_pp;
        }
        concat_len += strlen(gametype_pp) + sep_len + 2;

        int chars_required = snprintf(NULL, 0, "%d\0", *svr_players) + 1;
        char* players = (char*)malloc(chars_required);
        snprintf(players, chars_required, "%d\0", *svr_players);
        concat_len += chars_required;

        int sv_maxclients_chars_required = atoi(Info_ValueForKey(cs0, "sv_maxclients"));
        snprintf(NULL, 0, "%d\0", sv_maxclients_chars_required);
        char* sv_maxclients = (char*)malloc(sv_maxclients_chars_required + 1);
        snprintf(sv_maxclients, sv_maxclients_chars_required, "%d\0", sv_maxclients_chars_required);
        concat_len += sv_maxclients_chars_required + 1;

	    buffer = (char*)realloc(buffer, concat_len);
        if (buffer == NULL) {
            return -1;
        }
        buffer[0] = '\0';
        strcat(buffer, state);
        strcat(buffer, sep);
        strcat(buffer, hostname_pp);
        strcat(buffer, sep);
        strcat(buffer, mapname);
        strcat(buffer, sep);
        strcat(buffer, picture_name);
        strcat(buffer, sep);
        strcat(buffer, gametype_pp);
        strcat(buffer, sep);
        strcat(buffer, players);
        strcat(buffer, sep);
        strcat(buffer, sv_maxclients);
        strcat(buffer, sep);
        strcat(buffer, "\n\0");

        if (!WriteFile(write_handle, buffer, concat_len, &written, NULL)) {
            return -1;
        }

        Sleep(1000);
    }
    return 0;
}

int CL_StartDiscord() {
    const char* appdata_dir_path = getenv("appdata");

    if (appdata_dir_path == NULL) {
        return -1;
    }

    const char* dest_dir_name = "\\cod2rpc\\";
    int appdata_dir_path_len = strlen(appdata_dir_path);
    int dest_dir_name_len = strlen(dest_dir_name);

    size_t dest_path_len = strlen(appdata_dir_path) + strlen(dest_dir_name) + 1;
    char* dest_path = (char*)malloc(dest_path_len);
    if (dest_path == NULL) {
        // Handle memory allocation failure
        return -1;
    }

    dest_path[0] = '\0';
    strcat(dest_path, appdata_dir_path);
    strcat(dest_path, dest_dir_name);

    if (CreateDirectoryA(dest_path, 0) == ERROR_PATH_NOT_FOUND) {
        free(dest_path);  // Release allocated memory
        return -1;
    }

    const char* executable_name = "rpc.exe";
    dest_path = (char*)realloc(dest_path, strlen(executable_name) + appdata_dir_path_len + dest_dir_name_len + 1);
    if (dest_path == NULL) {
        // Handle memory allocation failure
        return -1;
    }
    dest_path[appdata_dir_path_len + dest_dir_name_len] = '\0';
    strcat(dest_path, executable_name);

    FILE* file = fopen(dest_path, "wb");

    if (file == NULL) {
        return -1;
    }

    if (fwrite(discord_exe, sizeof(unsigned char), discord_exe_len, file) != discord_exe_len) {
        fclose(file);
        return -1;
    }

    fclose(file);

    HANDLE discord_thread = CreateThread(NULL, 0, discord_communicate, dest_path, 0, NULL);

    if (discord_thread == NULL) {
        free(dest_path);  // Release allocated memory
        return -1;
    }
    free(dest_path);

    return 0;
}