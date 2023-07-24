#pragma once
#include <MQTTBroker/MQTTBroker.hpp>
#include <MSWorker/MSWorker.hpp>
#include "../GlobalModules/Config/Config.hpp"
#define DEFAULT_PORT "1883"



class MQTTWorker {
	
private:
	boost::asio::io_context __io_ctx;
	std::shared_ptr<MQTT_NS::server<>> __mqtt_server;
	std::shared_ptr<mqtt_broker::MQTTBroker> __mqtt_broker;
	std::shared_ptr<MSWorker> __ms_worker;
	string __config_file_name;
	map<string, string> __config_data;

	std::shared_ptr<Config> __configer;
	std::shared_ptr<Log> __logger;
	static const int __COUNT_CONFIG_FIELDS = 8;
	const vector<string> __CONFIG_FIELDS = {"Id","Main_server_ip", "Main_server_port",
												  "Main_server_info_port","BD_ip","BD_port",
												  "BD_login","BD_password"};
public:
	MQTTWorker(string config_file_name);
	~MQTTWorker();
	int init();
	void start();
	void stop();

	enum PROCESS_CODE {
		SUCCESSFUL = 0,
		CONFIG_FILE_NOT_OPEN,
		CONFIG_DATA_NOT_FULL
	};
};