#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include "Modules/WorkerServer/WorkerServer.hpp"
#include "../GlobalModules/Log/Log.hpp"
#include "../GlobalModules/Config/Config.hpp"
#include "Modules/HTTPSServer/HTTPSServer.hpp"
#include "Modules/SSLSertificateLocalhost/Sertificate.hpp"
#include "../GlobalModules/ClientDB/ClientDB.hpp"
#include "Modules/NetRepeater/NetRepeater.hpp"

using namespace std;

class MainServer:public std::enable_shared_from_this<MainServer> {
private:
	const int __TIME_UPDATE = 30;
	string __db_ip = "";
	string __db_login = "";
	string __db_password = "";
	int __port_db = 0;
	int __port_marusia_station = 0;
	int __port_mqtt = 0;
	int __port_worker_mqtt = 0;
	int __port_worker_mqtt_info = 0;
	int __port_worker_marusia = 0;
	std::vector<std::thread> __threads;

	shared_ptr<boost::asio::io_context> __io_ctx;
	shared_ptr<boost::asio::ssl::context> __ssl_ctx;
	shared_ptr<boost::asio::deadline_timer> __update_timer;
 	short __count_threads;

	map<string, string> __configuration;
	shared_ptr<Log> __logger;
	shared_ptr<Config> __configer;

	shared_ptr<map<string, vector<string>>> __sp_db_worker_marusia;
	shared_ptr<map<string, vector<string>>> __sp_db_worker_lu;
	shared_ptr<map<string, vector<string>>> __sp_db_marusia_station;
	shared_ptr<map<string, vector<string>>> __sp_db_lift_blocks;

	shared_ptr<https_server::Listener> __server_https;
	shared_ptr<worker_server::Server> __server_w_mqtt;
	shared_ptr<worker_server::Server> __server_w_marusia;
	shared_ptr<net_repeater::Server> __server_mqtt_repeater;
	shared_ptr<ClientDB> __client_db;

	queue<string> __table_name;
	queue<vector<string>> __table_fields;
	queue<string> __table_conditions;

	void __loadDataBase();
	void __startServers(map<string, map<string, vector<string>>> data);
	void __updateData(map<string, map<string, vector<string>>> &data);
	void __updateDataCallback(map<string, map<string, vector<string>>> data);
	void __updateTimerCallback(const boost::system::error_code& error);
	
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

int main(int argc, char* argv[]) {


	// Run the I/O service on the requested number of threads

	setlocale(LC_ALL, "Russian");
	string config_file_name = "";
	for (int i = 1; i < argc; i++) {
		string flags = argv[i];
		if (flags == "-cf" || flags == "--config_file") {
			config_file_name = argv[++i];
		}
	}

	MainServer ms;
	if (ms.init(config_file_name) != MainServer::PROCESS_CODE::SUCCESSFUL) {
		return -1;
	}
	ms.start();


	return EXIT_SUCCESS;
}