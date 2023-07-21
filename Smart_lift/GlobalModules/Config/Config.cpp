#include "Config.hpp"


Config::Config(shared_ptr<Log> lg, string root_directory, string way, string file_name)
{
	setLog(lg);

	__root_directory = root_directory;
	__way = way;
	if (file_name != "")
	{
		__file_name = file_name;
	}
	else
	{
		__file_name = CONFIG_FILE;
	}
	__final_path = __root_directory + __way +  __file_name;
	ifstream fin(__final_path);
	if (!fin.is_open())
	{
		__writeError(NOT_FOUND_CONFIG_FILE);
	}
	else
	{
		fin.close();
	}
}

void Config::setLog(shared_ptr<Log> lg)
{
	__log_ñonfig = lg;
}

void Config::setWay(string way)
{
	__way = way;
	__final_path = __way + __root_directory + __file_name;
}
void Config::setRootDirectory(string root_directory) {
	__root_directory = root_directory;
	__final_path = __way + __root_directory + __file_name;
}
void Config::setFileName(string file_name) {
	__file_name = file_name;
	__final_path = __way + __root_directory + __file_name;
}
void Config::__writeError(int error)
{
	__log_ñonfig->writeLog(error, "Config", "open config");
}

void Config::readConfig()
{
	ifstream fin(__final_path);
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
		__log_ñonfig->writeLog(0, "config", "Write Config");
		__config_info = config;
	}
	else
	{
		__writeError(NOT_FOUND_CONFIG_FILE);
		return;
	}
}
map<string, string> Config::getConfigInfo()
{
	return __config_info;
}