#include "stdheader.h"
#include "storage.h"

const char * storage::get_appdata_folder(char * szPath, int dwLen, bool create)
{
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath))) {
		strncpy(szPath, szPath, dwLen);
		szPath[dwLen] = 0;
		strcat(szPath, "\\" MOD_NAME);

		if (!create)
			return szPath;

		if (!PathFileExistsA(szPath)) {
			if (ERROR_SUCCESS != SHCreateDirectoryExA(NULL, szPath, NULL)) {
				return NULL;
			}

		}
	}
	return szPath;
}

bool storage::read_file(const std::string& fn, std::vector<char>& data)
{
	std::ifstream in(fn);

	if (!in.is_open())
		return false;
	in.seekg(0, in.end);
	size_t fs = in.tellg();
	in.seekg(0, in.beg);

	data.resize(fs);
	in.read(data.data(), fs);
	in.close();
	return true;
}

bool storage::file_exists(const std::string& fn)
{
	DWORD attr = GetFileAttributesA(fn.c_str());

	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

void storage::write_data_to_file(const std::string &fn, unsigned char *b, size_t l)
{
	FILE *fp = fopen(fn.c_str(), "wb");
	if (!fp)
		return;
	fwrite(b, 1, l, fp);
	fclose(fp);
}
