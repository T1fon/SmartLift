#pragma once

#include "Modules/Server/DataBaseServer.hpp"
#define DEFINE_CONFIG "config.txt"

class ServerDataBase: public enable_shared_from_this<ServerDataBase>
{
private:
	shared_ptr<net::io_context> __ioc;
	short __countThreads;
	shared_ptr<Server> __server;
	shared_ptr<Config> __config;
	shared_ptr<Log> __log_server;
	string __name_config;
	map<string, string> __config_info;
	static const int CONFIG_NUM_FIELDS = 2;
	vector<string> CONFIG_FIELDS = { "Port", "Count_threads"};
public:
	ServerDataBase(string config_way, string name_conf)
	{
		string way = "";
		if (name_conf == "")
		{
			__name_config = DEFINE_CONFIG;
		}
		else
		{
			__name_config = name_conf;
		}
		if (config_way != "")
		{
			way = config_way;
		}
		__log_server = make_shared<Log>("Log/", "./", "DataBase");
		__config = make_shared<Config>(__log_server, "./", way, __name_config);
		__config->readConfig();
	}
	~ServerDataBase()
	{
		stop();
	}
	void start()
	{
		__config_info = __config->getConfigInfo();
		if (__config_info.size() == 0)
		{
			__log_server->writeLog(1, "DataBase", "Config_File_not_open");
			return;
		}
		else if (__config_info.size() < CONFIG_NUM_FIELDS)
		{
			__log_server->writeLog(1, "DataBase", "Config_File_not_full");
		}
		try
		{
			for (size_t i = 0, length = __config_info.size() - 1; i < length; i++)
			{
				__config_info.at(CONFIG_FIELDS.at(i));
				cerr << CONFIG_FIELDS.at(i) << " " << __config_info.at(CONFIG_FIELDS.at(i)) << endl;
			}
		}
		catch (exception& e)
		{
			cerr << "error" << endl;
			__log_server->writeLog(1, "DataBase", e.what());
			return;
		}
		cerr << "0" << endl;
		__countThreads = stoi(__config_info.at("Count_threads"));
		cerr << "1" << endl;
		__ioc = make_shared<boost::asio::io_context>(__countThreads);
		cerr << "2" << endl;
		__server = make_shared<Server>(__ioc, "", __log_server, __config_info);
		cerr << "3" << endl;
		__server->run();

		std::vector<std::thread> v;
		v.reserve(__countThreads - 1);
		for (auto i = __countThreads - 1; i > 0; --i)
			v.emplace_back(
				[this]
				{
					__ioc->run();
				});
		__ioc->run();
	}
	void stop()
	{
		__server->stop();
	}

};