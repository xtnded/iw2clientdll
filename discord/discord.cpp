#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../qcommon/qcommon.h"
#include "discord.h"
#include "../client_mp/client.h"

DWORD WINAPI discord_integrate(
	LPVOID lp_param
) {
	HANDLE write_handle = (HANDLE)lp_param;
	DWORD written;
	char buff[903] = { 0 };

	while (1) {
		const char* state;
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
		
		const char* hostname = "\0";
		if (
			!(*cls_state == CA_DISCONNECTED
			|| *cls_state == CA_CONNECTED
			|| *cls_state == CA_CONNECTING
			|| *cls_state == CA_CHALLENGING)
		) {
			const char* hostname_ptr = Info_ValueForKey(cs0, "sv_hostname");
			char tmp_hostname[COL_SIZE] = { 0 };
			Q_strncpyz(tmp_hostname, hostname_ptr, sizeof(tmp_hostname));
			hostname = Com_CleanHostname(tmp_hostname, false);
		}

		const char* map_name_ptr = Info_ValueForKey(cs0, "mapname");
		char map_name[COL_SIZE] = { 0 };
		Q_strncpyz(map_name, map_name_ptr, sizeof(map_name));

		const char* clean_map_name_ptr = Info_ValueForKey(cs0, "mapname");
		char clean_map_name[COL_SIZE] = { 0 };
		Q_strncpyz(clean_map_name, clean_map_name_ptr, sizeof(clean_map_name));
		
		Com_CleanMapname(clean_map_name);

		const char *picture_name_ptr;
		if (map_name[0] == '\0') {
			picture_name_ptr = "main_small\0";
		}
		else {
			picture_name_ptr = "main_small\0";
		}

		const char* gametype_pp = "\0";
		const char* gametype_p = Info_ValueForKey(cs0, "g_gametype");

		char gametype[COL_SIZE] = { 0 };
		Q_strncpyz(gametype, gametype_p, sizeof(gametype));

		if (*cls_state == CA_ACTIVE) {
			gametype_pp = Com_GametypeName(gametype, false);
			state = gametype_pp;
		}

		int chars_required = snprintf(NULL, 0, "%d\0", *svr_players) + 1;
		char players[COL_SIZE] = { 0 };
		snprintf(players, chars_required, "%d\0", *svr_players);

		const char* sv_maxclients = Info_ValueForKey(cs0, "sv_maxclients");

		strcat(buff, state);
		strcat(buff, "\t\0");

		strcat(buff, hostname);
		strcat(buff, "\t\0");

		strcat(buff, clean_map_name_ptr);
		strcat(buff, "\t\0");

		strcat(buff, picture_name_ptr);
		strcat(buff, "\t\0");

		strcat(buff, gametype_pp);
		strcat(buff, "\t\0");

		strcat(buff, players);
		strcat(buff, "\t\0");

		strcat(buff, sv_maxclients);
		strcat(buff, "\n\0");

		if (!WriteFile(write_handle, buff, strlen(buff), &written, NULL)) {
			return -1;
		}

		buff[0] = '\0';

		Sleep(1250);
	}
}

int CL_StartDiscord(void) {
	const char* temp_path = getenv("temp");
	if (temp_path == NULL) {
		return -1;
	}

	HANDLE read_handle, write_handle;
	char* buffer = NULL;

	PROCESS_INFORMATION pi;
	STARTUPINFOA si;

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

	short int ret_code = 0;
	FILE* dest_file = NULL;

	const char* exe_name = "COD2DiscordIntegration.exe\0";
	size_t len = strlen(temp_path) + strlen(exe_name) + 2;
	
	char* dest_path = (char*)calloc(len, sizeof(const char));

	if (dest_path == NULL) {
		return -1;
	}

	if (snprintf(dest_path, len, "%s\\%s", temp_path, exe_name) != (len - 1)) {
		goto clean;
	}
	
	dest_file = fopen(dest_path, "wb");

	if (dest_file == NULL) {
		ret_code = -1;
		goto clean;
	}

	if ((fwrite(discord_exe, sizeof(unsigned char), discord_exe_len, dest_file) != discord_exe_len) || (fclose(dest_file) != 0)
	) {
		ret_code = -1;
		goto clean;
	}

	fclose(dest_file);

	if (!CreateProcessA(NULL, dest_path, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		ret_code = -1;
		goto clean;
	}

	/*HANDLE discord_thread = CreateThread(NULL, 0, discord_communicate, dest_path, 0, NULL);*/
	if (CreateThread(NULL, 0, discord_integrate, (LPVOID)write_handle, 0, NULL) == NULL) {
		goto clean;
	}

	clean:
	free(dest_path);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return ret_code;
}