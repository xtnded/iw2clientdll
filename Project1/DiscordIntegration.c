#pragma comment(lib, "discord-rpc.lib")

#include <windows.h>
#include <stdio.h>

#include "discord_rpc.h"

#define BUFF_SIZE 878
#define COL_SIZE 128
#define MSG_COLS 7

short int convtl_prcd(
	char *str_ptr,
	long *result
) {
	char *ptr;
	long num = strtol(str_ptr, &ptr, 10);
	if (ptr == str_ptr) {
		return -1;
	}
	result = &num;
	return 0;
}

int main(void) {
	FreeConsole();

	DiscordEventHandlers handlers;
	memset(&handlers, 0x0, sizeof(handlers));
	Discord_Initialize("1109509865279332392", &handlers, 1, NULL);
	
	short int readed_bytes;
	
	char *conv_ptr = NULL;
	char buff[BUFF_SIZE];
	char msg[MSG_COLS][COL_SIZE] = { 0 };
	
	long* party_size = NULL, * party_max = NULL;
	DiscordRichPresence presence;

	while (
		(readed_bytes = _read(0, buff, BUFF_SIZE)) > 0
	) {
		memset(&presence, 0, sizeof(presence));

		buff[readed_bytes - 1] = '\0';
		//printf("%s\n", buff);
		unsigned short int sep_counter = 0, msg_index = 0;

		for (
			unsigned short int buff_index = 0;
			buff_index < readed_bytes;
			buff_index++
		) {
			if (buff[buff_index] == '\t') {
				msg[sep_counter][msg_index] = '\0';
				msg_index = 0; sep_counter++;
				msg[sep_counter][msg_index] = '\0';
				continue;
			}
			msg[sep_counter][msg_index] = buff[buff_index];
			msg_index++;
		}

		// if the msg has been corrupted -> continue 
		if (sep_counter != 6) {
			buff[0] = '\0';
			continue;
		}

		msg[sep_counter][msg_index] = '\0';
		
		//for (int x = 0; x < 7; x++) {
		//	printf("%d => %s\n", x, msg[x]);
		//	// Na koñcu robi siê jakiœ chuj
		//}
		//printf("\n");

		presence.state = msg[0];
		presence.details = msg[1];
		presence.largeImageText = msg[2];
		presence.largeImageKey = msg[3];

		if (convtl_prcd(msg[5], party_size) ||convtl_prcd(msg[6], party_max)
		) {
			goto send_data;
		}

		if (0 < *party_size < 65) {
			presence.partySize = party_size;
			presence.partyMax = party_max;
		}

		send_data:
		buff[0] = '\0';
		Discord_UpdatePresence(&presence);
	}

	return 0;
}