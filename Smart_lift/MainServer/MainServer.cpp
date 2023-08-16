#include "MainServer.hpp"

MainServer::MainServer() {}
MainServer::~MainServer() {
	stop();
}
MainServer::PROCESS_CODE MainServer::init(string path_to_config_file) {

	__logger = make_shared<Log>("","./","MainServer");
	__configer = make_shared<Config>(__logger, "./", path_to_config_file);
	__configer->readConfig();
	map<string, string> configuration = __configer->getConfigInfo();
	try {
		if (configuration.size() == 0) {
			return CONFIG_FILE_NOT_OPEN;
		}
		__db_ip = configuration.at("DB_ip");
		__db_login = configuration.at("DB_login");
		__db_password = configuration.at("DB_password");
		__port_db = stoi(configuration.at("DB_port"));
		__port_marusia_station = stoi(configuration.at("Marusia_port"));
		__port_mqtt = stoi(configuration.at("MQTT_port"));
		__port_worker_mqtt = stoi(configuration.at("Worker_MQTT_port"));;
		__port_worker_mqtt_info = stoi(configuration.at("Worker_MQTT_info_port"));
		__port_worker_marusia = stoi(configuration.at("Worker_marusia_port"));
		__count_threads = stoi(configuration.at("Count_threads"));
		if (__port_marusia_station < 1 || __port_mqtt < 1 || __port_worker_mqtt < 1 ||
			__port_worker_mqtt_info < 1 || __port_worker_marusia < 1 || __count_threads < 1) 
		{
			throw exception("Port <= 0");
		}
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		return PROCESS_CODE::CONFIG_DATA_NOT_FULL;
	}
	
	__io_ctx = make_shared<boost::asio::io_context>(__count_threads);
	__ssl_ctx = make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
	load_server_certificate(*__ssl_ctx);
	__server_w_mqtt = make_shared<worker_server::Server>(__io_ctx, __port_worker_mqtt_info,worker_server::WORKER_MQTT_T);
	__server_w_marusia = make_shared<worker_server::Server>(__io_ctx, __port_worker_marusia, worker_server::WORKER_MARUSIA_T);
	__server_https = make_shared<https_server::Listener>(*__io_ctx, *__ssl_ctx,__port_marusia_station, __server_w_mqtt->getSessions(), __server_w_marusia->getSessions());
	__client_db = std::make_shared<ClientDB>(__db_ip, to_string(__port_db), __db_login, __db_password, 
										"Main_server", std::make_shared<boost::asio::ip::tcp::socket>(*__io_ctx), 
										bind(&MainServer::__updateDataCallback, this,_1));
	__update_timer = make_shared<boost::asio::deadline_timer>(*__io_ctx);
	return PROCESS_CODE::SUCCESSFUL;
}
void MainServer::stop(){
	__server_w_mqtt->stop();
	__server_w_marusia->stop();
}
void MainServer::start() {
	
	__loadDataBase();
	
	/*std::vector<std::thread> v;
	v.reserve(__count_threads - 1);
	for (auto i = __count_threads - 1; i > 0; --i)
		v.emplace_back(
			[this]
			{
				__io_ctx->run();
			});*/
	__io_ctx->run();
}
void MainServer::__startServers(map<string, map<string, vector<string>>> data) {
	cout << "Start servers" << endl;
	
	try {
		__updateData(data);
	}
	catch (exception& e) {
		cerr << "StartServers: " << e.what();
		return;
	}
	/*��������� ������ �� ���������� ������*/
	__client_db->setCallback(bind(&MainServer::__updateDataCallback, this, _1)); 
	__update_timer->expires_from_now(boost::posix_time::seconds(__TIME_UPDATE));
	__update_timer->async_wait(boost::bind(&MainServer::__updateTimerCallback, this, _1));

	__server_w_mqtt->start(make_shared<shared_ptr<map<string, vector<string>>>>(__sp_db_worker_lu));
	__server_w_marusia->start(make_shared<shared_ptr<map<string, vector<string>>>>(__sp_db_worker_marusia));
	__server_https->start(make_shared<shared_ptr<map<string, vector<string>>>>(__sp_db_marusia_station),
						  make_shared<shared_ptr<map<string, vector<string>>>>(__sp_db_lift_blocks));
}
void MainServer::__loadDataBase() {
	__client_db->setCallback(bind(&MainServer::__startServers, this, _1));
	
	__table_name.push("WorkerMarussia");
	__table_fields.push({"WorkerMId"});
	__table_conditions.push("");
	
	__table_name.push("WorkerLU");
	__table_fields.push({ "WorkerLuId" });
	__table_conditions.push("");

	__table_name.push("MarussiaStation");
	__table_fields.push({ "ApplicationId", "WorkerId", "WokerSecId", "LiftBlockId"});
	__table_conditions.push("");

	__table_name.push("LiftBlocks");
	__table_fields.push({ "LiftId", "WorkerLuId", "WorkerLuSecId", "Descriptor"});
	__table_conditions.push("");

	__client_db->setQuerys(__table_name, __table_fields, __table_conditions);
	__client_db->start();
}
void MainServer::__updateData(map<string, map<string, vector<string>>> &data) {
	shared_ptr<map<string, vector<string>>> temp_sp_db_worker_marusia = make_shared<map<string, vector<string>>>(data.at("WorkerMarussia"));
	shared_ptr<map<string, vector<string>>> temp_sp_db_worker_lu = make_shared<map<string, vector<string>>>(data.at("WorkerLU"));
	shared_ptr<map<string, vector<string>>> temp_sp_db_marusia_station = make_shared<map<string, vector<string>>>(data.at("MarussiaStation"));
	shared_ptr<map<string, vector<string>>> temp_sp_db_lift_blocks = make_shared<map<string, vector<string>>>(data.at("LiftBlocks"));

	__sp_db_worker_marusia = temp_sp_db_worker_marusia;
	__sp_db_worker_lu = temp_sp_db_worker_lu;
	__sp_db_marusia_station = temp_sp_db_marusia_station;
	__sp_db_lift_blocks = temp_sp_db_lift_blocks;
}
void MainServer::__updateDataCallback(map<string, map<string, vector<string>>> data){
	try {
		__updateData(data);
	}
	catch (exception& e) {
		cerr << "__updateDataCallback: " << e.what();
		return;
	}
}
void MainServer::__updateTimerCallback(const boost::system::error_code& error){
	__client_db->setQuerys(__table_name, __table_fields, __table_conditions);
	__client_db->start();
	__update_timer->expires_from_now(boost::posix_time::seconds(__TIME_UPDATE));
	__update_timer->async_wait(boost::bind(&MainServer::__updateTimerCallback, this,_1));
}