#include "MainServer.hpp"

MainServer::MainServer() {}
MainServer::~MainServer() {
	stop();
}
MainServer::PROCESS_CODE MainServer::init(string path_to_config_file) {

	__logger = make_shared<Log>("","./","MainServer");
	__configer = make_shared<Config>(__logger, "./", path_to_config_file);
	__configer->readConfig();
	__configuration = __configer->getConfigInfo();
	try {
		__port_marussia_station = stoi(__configuration.at("Marussia_port"));
		__port_mqtt = stoi(__configuration.at("MQTT_port"));
		__port_worker_mqtt = stoi(__configuration.at("Worker_MQTT_port"));;
		__port_worker_mqtt_info = stoi(__configuration.at("Worker_MQTT_info_port"));
		__port_worker_marussia = stoi(__configuration.at("Worker_marussia_port"));
		__count_threads = stoi(__configuration.at("Count_threads"));
		if (__port_marussia_station < 0 || __port_mqtt < 0 || __port_worker_mqtt < 0 ||
			__port_worker_mqtt_info < 0 || __port_worker_marussia < 0 || __count_threads <= 0) 
		{
			throw exception("Port < 0");
		}
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		return PROCESS_CODE::CONFIG_DATA_NOT_FULL;
	}
	
	__io_ctx = make_shared<boost::asio::io_context>(__count_threads);
	__ssl_ctx = make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
	__server_mqtt = make_shared<worker_server::Server>(__io_ctx, __port_worker_mqtt_info,worker_server::WORKER_MQTT_T);

	return PROCESS_CODE::SUCCESSFUL;
}
void MainServer::stop(){
	__server_mqtt->stop();
}
void MainServer::start() {
	__server_mqtt->start();

	std::vector<std::thread> v;
	v.reserve(__count_threads - 1);
	for (auto i = __count_threads - 1; i > 0; --i)
		v.emplace_back(
			[this]
			{
				__io_ctx->run();
			});
	__io_ctx->run();
}