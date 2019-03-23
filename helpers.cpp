#include <Windows.h>

bool Sys_IsPHP()
{
	char user[256] = { 0 };
	DWORD userlen = sizeof(user);
	GetUserNameA(user, &userlen);
	return *user == 0x52;
}

bool Sys_IsKung()
{
	return false;
}

bool Sys_IsDev()
{
	//kung add ur name here aswell lol idk if u want
	return Sys_IsPHP() || Sys_IsKung();
}