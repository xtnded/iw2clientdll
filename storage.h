#pragma once
#include <vector>

namespace storage
{
	const char *get_appdata_folder(char *szPath, int dwLen, bool create);
	bool file_exists(const std::string&);
	void write_data_to_file(const std::string&, unsigned char*, size_t);
	bool read_file(const std::string&, std::vector<char>&);
};