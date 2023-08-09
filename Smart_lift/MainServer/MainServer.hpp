#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include "Modules/WorkerServer/WorkerServer.hpp"
#include "../GlobalModules/Log/Log.hpp"
#include "../GlobalModules/Config/Config.hpp"
#include "Modules/HTTPSServer/HTTPSServer.hpp"
#include "Modules/SSLSertificateLocalhost/Sertificate.hpp"

using namespace std;

class MainServer {
private:
	short __port_marussia_station = 0;
	short __port_mqtt = 0;
	short __port_worker_mqtt = 0;
	short __port_worker_mqtt_info = 0;
	short __port_worker_marussia = 0;

	shared_ptr<boost::asio::io_context> __io_ctx;
	shared_ptr<boost::asio::ssl::context> __ssl_ctx;
	short __count_threads;

	map<string, string> __configuration;
	shared_ptr<Log> __logger;
	shared_ptr<Config> __configer;

	shared_ptr<https_server::Listener> __server_https;
	shared_ptr<worker_server::Server> __server_w_mqtt;
	shared_ptr<worker_server::Server> __server_w_marussia;

public:
	enum PROCESS_CODE {
		SUCCESSFUL = 0,
		CONFIG_FILE_NOT_OPEN,
		CONFIG_DATA_NOT_FULL
	};

	MainServer();
	~MainServer();

	PROCESS_CODE init(string path_to_config_file);
	void start();
	void stop();
};