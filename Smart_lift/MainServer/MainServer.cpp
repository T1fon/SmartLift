#include "MainServer.hpp"

MainServer::MainServer(map<string, string> configuration) {
	__port_marussia_station = 443;
	__port_mqtt = 1883;
	__port_worker_mqtt = 1833;
	__port_worker_mqtt_info = 1337;
	__port_worker_marussia = 0;
}
MainServer::~MainServer() {
	stop();
}
void MainServer::init(unsigned short count_threads) {
	__count_threads = count_threads;
	__io_ctx = make_shared<boost::asio::io_context>(count_threads);
	__ssl_ctx = make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
	__server_mqtt = make_shared<worker_server::Server>(__io_ctx, __port_worker_mqtt_info,worker_server::WORKER_MQTT_T);
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