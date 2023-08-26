#pragma once

#include "Modules/Worker/Worker.hpp"
#define DEFINE_CONFIG "config.txt"

class ServerWorker : public enable_shared_from_this<ServerWorker>
{
public:
	ServerWorker(string name_conf_file)
	{
		__log_server = make_shared<Log>("Log/", "./", "Marussia_Worker");
		if (name_conf_file == "")
		{
			name_config = DEFINE_CONFIG;
		}
		else
		{
			name_config = name_conf_file;
		}
		__ioc = make_shared<boost::asio::io_context>(__count_threads);
		__config = make_shared<Config>(__log_server, "./", "", name_config);
		__config->readConfig();
		__config_info = __config->getConfigInfo();
	}
	void run()
	{
		if (__config_info.size() == 0)
		{
			__log_server->writeLog(1, "Marussia_Worker", "Config_File_not_open");
			return;
		}
		else if (__config_info.size() < CONFIG_NUM_FIELDS)
		{
			__log_server->writeLog(1, "Marussia_Worker", "Config_File_not_full");
			return;
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
			__log_server->writeLog(1, "Marussia_Worker", e.what());
			return;
		}
		try
		{
			__count_threads = stoi(__config_info.at("count_threads"));
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}

		__session = make_shared<Worker>(__config_info, *__ioc, __log_server);
		__session->start();
		std::vector<std::thread> v;
		v.reserve(__count_threads - 1);
		for (auto i = __count_threads - 1; i > 0; --i)
			v.emplace_back(
				[this]
				{
					__ioc->run();
				});
		__ioc->run();
	}
	void stop()
	{
		__session->stop();
	}
	~ServerWorker()
	{
		this->stop();
	}
private:
	shared_ptr<Log> __log_server;
	shared_ptr<Config> __config;
	map<string, string> __config_info;
	shared_ptr<tcp::socket> __sock;
	static const int CONFIG_NUM_FIELDS = 1;
	vector<string> CONFIG_FIELDS = { "Id", "Main_server_ip", "Main_server_port", "BD_ip", "BD_port", "BD_login", "BD_password", "count_threads"};
	int __count_threads;
	shared_ptr<net::io_context> __ioc;
	shared_ptr<Worker> __session;
	string name_config;

};
