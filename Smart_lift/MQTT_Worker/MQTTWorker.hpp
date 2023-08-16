#pragma once
#include <vector>
#include <MQTTBroker/MQTTBroker.hpp>
#include <MSWorker/MSWorker.hpp>
#include "../GlobalModules/Config/Config.hpp"
#include "../GlobalModules/ClientDB/ClientDB.hpp"
#define DEFAULT_PORT "1883"


class MQTTWorker: public std::enable_shared_from_this<MQTTWorker> {
	
private:
	const int __TIME_UPDATE = 100;
	std::shared_ptr<boost::asio::io_context> __io_ctx;
	std::shared_ptr<MQTT_NS::server<>> __mqtt_server;
	std::shared_ptr<mqtt_broker::MQTTBroker> __mqtt_broker;
	std::shared_ptr<MSWorker> __ms_worker;
	mqtt_broker::con_sp_t __mqtt_worker;
	std::vector<std::thread> __v;
	vector<string> __lu_ids = {};

	std::shared_ptr<Config> __configer;
	std::shared_ptr<Log> __logger;
	string __config_file_name;

	const vector<string> __CONFIG_FIELDS = {"Id","Main_server_ip", "Main_server_port",
												  "Main_server_info_port","DB_ip","DB_port",
												  "DB_login","DB_password","Count_threads"};
	map<string, string> __config_data;
	string __id = "";
	int __count_threads = 1;
	string __main_server_ip;
	int __main_server_port;
	int __main_server_info_port;
	string __db_ip;
	int __db_port;
	string __db_login;
	string __db_password;

	shared_ptr<ClientDB> __client_db;

	queue<string> __table_name;
	queue<vector<string>> __table_fields;
	queue<string> __table_conditions;
	shared_ptr<map<string, vector<string>>> __sp_db_lift_blocks;
	shared_ptr<map<string, string>> __sp_db_map_lb_descriptor;
	shared_ptr<map<string, string>> __sp_db_map_login_password;
	shared_ptr<boost::asio::deadline_timer> __update_timer;

	void __loadDataBase();
	void __startWorkers(map<string, map<string, vector<string>>> data);
	void __updateData(map<string, map<string, vector<string>>>& data);
	void __updateDataCallback(map<string, map<string, vector<string>>> data);
	void __updateTimerCallback(const boost::system::error_code& error);

public:
	MQTTWorker(string config_file_name);
	~MQTTWorker();
	int init();
	void start();
	void stop();
	void moveLift(string lu_description, string floor_number, string station_id);

	enum PROCESS_CODE {
		SUCCESSFUL = 0,
		CONFIG_FILE_NOT_OPEN,
		CONFIG_DATA_NOT_FULL
	};
};