#pragma once

#include "Modules/Server/DataBaseServer.hpp"

class ServerDataBase: public enable_shared_from_this<ServerDataBase>
{
private:
	shared_ptr<net::io_context> __ioc;
	short __countThreads;
	shared_ptr<Server> __server;
	shared_ptr<Config> __config;
	shared_ptr<Log> __log_server;
	map<string, string> __config_info;
	static const int CONFIG_NUM_FIELDS = 1;
	vector<string> CONFIG_FIELDS = { "port", "count_threads"};
public:
	ServerDataBase()
	{
		__ioc = make_shared<boost::asio::io_context>(__countThreads);
		__log_server = make_shared<Log>("Log/", "./", "DataBase");
		__config = make_shared<Config>(__log_server, "./", "", "");
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
			for (size_t i = 0, length = __config_info.size(); i < length; i++)
			{
				__config_info.at(CONFIG_FIELDS.at(i));
			}
		}
		catch (exception& e)
		{
			__log_server->writeLog(1, "DataBase", e.what());
			return;
		}
		__server = make_shared<Server>(__ioc, "", __log_server, __config_info);
		__countThreads = stoi(__config_info.at("count_threads"));
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