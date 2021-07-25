#pragma once
#include <sstream>
#include <fstream>
#include <unordered_map>

std::unordered_map<std::string, std::string> g_config;
#define CONFIG_NAME std::string("name")
#define CONFIG_IP std::string("server_ip")
#define CONFIG_PORT std::string("server_port")

int read_cfg()
{
	int lines = 0;
	std::ifstream is_file("skymp_config.ini");

	std::string line;
	while (std::getline(is_file, line))
	{
		std::istringstream is_line(line);
		std::string key;
		if (std::getline(is_line, key, '='))
		{
			std::string value;
			if (std::getline(is_line, value))
			{
				g_config.insert({ key, value });
				++lines;
			}
		}
	}

	is_file.close();
	return lines;
}