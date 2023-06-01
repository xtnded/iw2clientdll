
#pragma comment(lib, "discord-rpc.lib")

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "discord_rpc.h"

#define BUFF_SIZE 4096

void DiscordInitialize() {
	DiscordEventHandlers handlers;
	memset(&handlers, 0x0, sizeof(handlers));
	Discord_Initialize("1109509865279332392", &handlers, 1, NULL);
}

int main(void) {
	//puts("Starting discord");
	DiscordInitialize();
	//puts("Discord initialized");
	FreeConsole();
	char buff[BUFF_SIZE + 1];
	char rpc[7][128] = { 0 };
	while (1) {
		DiscordRichPresence discord_presence;
		memset(&discord_presence, 0, sizeof(discord_presence));
		int readed_bytes = _read(0, buff, BUFF_SIZE);
		if (readed_bytes == 0) {
			MessageBoxA(0, "Error: Discord RPC has crashed", "ERROR", 0);
			return -1;
		}
		// Overwrite \n character with \0 (We do not need \n anymore)
		buff[readed_bytes - 1] = '\0';
		for (int i = 0, j = 0, r = 0; i < readed_bytes - 1; i++) {
			if (buff[i] == '\t') {
				rpc[r][j] = '\0';
				j = 0; r++;
				rpc[r][j] = '\0';
				continue;
			}
			rpc[r][j] = buff[i];
			j++;
		}
		/*
		0 = state
		1 = hostname
		2 = mapname
		3 = picture_name
		4 = GAMETYPE
		5 = players
		6 = max players
		*/
		discord_presence.state = rpc[0];
		discord_presence.details = rpc[1];
		discord_presence.largeImageText = rpc[2];
		discord_presence.largeImageKey = rpc[3];
		if (atoi(rpc[5]) > 0 && atoi(rpc[5]) < 65) {
			discord_presence.partySize = atoi(rpc[5]);
			discord_presence.partyMax = atoi(rpc[6]);
		}

		Discord_UpdatePresence(&discord_presence);
		//For debugging
		//for (int i = 0; i < 7; i++) {
		//	printf("%s\\t\t", rpc[i]);
		//	rpc[i][0] = '\0';
		//}
		//printf("\r\n");
	}
}