#include "Config.hpp"


Config::Config(shared_ptr<Log> lg, string boofWay)
{
	setLog(lg);
	setWay(boofWay);//указать путь относительно папки Smart_lift(DataBase\\Modules\\)файл config указывать не нужно
	__way = __way + __boofWay + __conf;
	ifstream fin(__way);
	if (!fin.is_open())
	{
		writeError(NOT_FOUND_CONFIG_FILE);
	}
	else
	{
		fin.close();
	}
}

void Config::setLog(shared_ptr<Log> lg)
{
	__logConfig = lg;
}

void Config::setWay(string boof)
{
	__boofWay = boof;
}

void Config::writeError(int error)
{
	__logConfig->writeLog(error, "Config", "open config");
}

void Config::readConfig()
{
	ifstream fin(__way);
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
		__logConfig->writeLog(0, "config", "Write Config");
		__configInfo = config;
	}
	else
	{
		writeError(NOT_FOUND_CONFIG_FILE);
		return;
	}
}
map<string, string> Config::getConfigInfo()
{
	return __configInfo;
}