#pragma once
#include <MQTTBroker/MQTTBroker.hpp>
#include <MSWorker/MSWorker.hpp>

#define DEFAULT_PORT "1883"



class MQTTWorker {
private:
	boost::asio::io_context __io_ctx;
	std::shared_ptr<MQTT_NS::server<>> __mqtt_server;
	mqtt_broker::MQTTBroker __mqtt_broker;
	std::shared_ptr<MSWorker> __ms_worker;
public:
	MQTTWorker();
	~MQTTWorker();
	void init();
	void start();
	void stop();
};