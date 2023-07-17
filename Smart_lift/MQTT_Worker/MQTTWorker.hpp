#pragma once
#include "Modules/MQTTBroker/MQTTBroker.hpp"
#define DEFAULT_PORT "1883"

class MQTTWorker {
private:
	boost::asio::io_context __io_ctx;
	shared_ptr<MQTT_NS::server<>> __mqtt_server;
	mqtt_broker::MQTTBroker __mqtt_broker;
public:
	MQTTWorker();
	~MQTTWorker();
	void init();
	void start();
	void stop();
};