#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include "Modules/WorkerServer/WorkerServer.hpp"

using namespace std;

class MainServer {
private:
	unsigned short __port_marussia_station = 0;
	unsigned short __port_mqtt = 0;
	unsigned short __port_worker_mqtt = 0;
	unsigned short __port_worker_mqtt_info = 0;
	unsigned short __port_worker_marussia = 0;

	shared_ptr<boost::asio::io_context> __io_ctx;
	shared_ptr<boost::asio::ssl::context> __ssl_ctx;
	unsigned short __count_threads;

	shared_ptr<worker_server::Server> __server_mqtt;

public:
	MainServer(map<string,string> configuration);
	~MainServer();

	void init(unsigned short count_threads = 1);
	void start();
	void stop();
};