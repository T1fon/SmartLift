#include "Config.hpp"


void Config::writeError(int error)
{
	logConfig.writeLog(error, "Config", "open config");
}

map <string, string> Config::readConfig()
{
	ifstream fin(way);
	if (fin.is_open())
	{
		map <string, string> config;
		while (!fin.eof())
		{
			string boof;
			string key;
			getline(fin, boof);
			size_t border = boof.find(":");
			key.append(boof, 0, border);
			boof.erase(0, border + 1);
			config.insert(pair<string, string>(key, boof));

		}
		logConfig.writeLog(0, "config", "Write Config");
		return config;
	}
	else
	{
		writeError(NOT_FOUND_CONFIG_FILE);
		map <string, string> config = { {"0", "0"} };
		logConfig.writeLog(0, "config", "Write Config");
		return config;
	}

}