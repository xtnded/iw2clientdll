#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stdheader.h"
#include "discord.h"
#include "client.h"

#pragma pack(push, 8)
#include "Discord/discord_game_sdk.h"
#pragma pack(pop)

struct Application {
	struct IDiscordCore *core;
	struct IDiscordUsers *users;
};

void test(void) {
	struct Application app;
	// Don't forget to memset or otherwise initialize your classes!
	memset(&app, 0, sizeof(app));
	struct IDiscordCoreEvents *events;
	memset(&events, 0, sizeof(events));
	struct DiscordCreateParams *params;
	params->client_id = *"1109509865279332392";
	params->flags = DiscordCreateFlags_Default;
	params->events = events;
	params->event_data = &app;

	DiscordCreate(DISCORD_VERSION, params, &app.core);
}

int CL_StartDiscord(void) {
	return 0;
}
