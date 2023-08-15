//(*end_p).publish("LU127728/set/cmd", std::string("{\"cmdlft\":[1,") + std::to_string(a.at(i)) + std::string("]}"));
#include "MQTTWorker.hpp"
#include <string>
#include <iostream>
using namespace std;

MQTTWorker::MQTTWorker(string config_file_name) {
    __config_file_name = config_file_name;
}
MQTTWorker::~MQTTWorker() {
    this->stop();
}
int MQTTWorker::init() {

    __logger = make_shared<Log>("", "test","MQTTWorker");
    __configer = make_shared<Config>(__logger,"./","");
    
    __configer->readConfig();
    __config_data = __configer->getConfigInfo();

    if (__config_data.size() == 0) {
        return CONFIG_FILE_NOT_OPEN;
    }

    try {
        __id = __config_data.at("Id");
        __count_threads = stoi(__config_data.at("Count_threads"));
        __main_server_ip = __config_data.at("Main_server_ip");
        __main_server_port = stoi(__config_data.at("Main_server_port"));
        __main_server_info_port = stoi(__config_data.at("Main_server_info_port"));
        __db_ip = __config_data.at("DB_ip");
        __db_port = stoi(__config_data.at("DB_port"));
        __db_login = __config_data.at("DB_login");
        __db_password = __config_data.at("DB_password");
        if (__main_server_port < 1 || __main_server_info_port < 1 || __db_port < 1 || __count_threads < 1) {
            throw exception("Port or count thread < 1");
        }
    }
    catch (exception& e) {
        cout << e.what() << endl;
        return CONFIG_DATA_NOT_FULL;
    }

    __sp_db_map_lb_descriptor = make_shared<map<string,string>>();
    __sp_db_map_login_password = make_shared<map<string, string>>();

    MQTT_NS::setup_log();
    __io_ctx = make_shared<boost::asio::io_context>(__count_threads);

    __mqtt_server = make_shared<MQTT_NS::server<>>(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            boost::lexical_cast<std::uint16_t>(__main_server_port)
        ),
        *__io_ctx
    );
    __mqtt_broker = make_shared<mqtt_broker::MQTTBroker>();
    __mqtt_broker->setServer(__mqtt_server);
    __mqtt_broker->init();

    __ms_worker = make_shared<MSWorker>(__main_server_ip, to_string(__main_server_info_port), __id, *__io_ctx);
    __ms_worker->setCallback(bind(&MQTTWorker::moveLift, this, _1, _2, _3));
    __mqtt_worker = __mqtt_broker->getWorker();

    __client_db = make_shared<ClientDB>(__db_ip, to_string(__db_port), __db_login, __db_password, "Main_server", make_shared<boost::asio::ip::tcp::socket>(*__io_ctx));
    __update_timer = make_shared<boost::asio::deadline_timer>(*__io_ctx);

    return SUCCESSFUL;
}
void MQTTWorker::start() {
    /*заполнение __lu_id*/
    //__mqtt_broker->start(make_shared<shared_ptr<map<string, string>>>(__sp_db_map_login_password));
    //__ms_worker->start(make_shared<shared_ptr<map<string, string>>>(__sp_db_map_lb_descriptor));

    
    __loadDataBase();
    /*------------------*/

    __v.reserve(__count_threads - 1);
    for (auto i = __count_threads - 1; i > 0; --i)
        __v.emplace_back(
            [this]
            {
                __io_ctx->run();
                //__io_ctx_mqtt->run();
            });

    __io_ctx->run();
    
}
void MQTTWorker::stop() {
    __mqtt_broker->stop();
    __ms_worker->stop();
}

void MQTTWorker::moveLift(string lu_description, string floor_number, string station_id) {
    __mqtt_worker->async_publish("LU" + lu_description +"/set/cmd", "{\"cmdlft\":[1," + floor_number + "]}");
    __ms_worker->successMove(station_id);
}
void MQTTWorker::__loadDataBase() {
    __client_db->setCallback(bind(&MQTTWorker::__startWorkers, this, _1));

    __table_name.push("LiftBlocks");
    __table_fields.push({ "LiftId", "Descriptor", "WorkerLuId", "WorkerLuSecId", "Login", "Password"});

    __client_db->setQuerys(__table_name, __table_fields);
    __client_db->start();
}
void MQTTWorker::__startWorkers(map<string, map<string, vector<string>>> data) {
    
    try {
        __updateData(data);
    }
    catch (exception& e) {
        cerr << "__startWorkers: " << e.what();
        return;
    }

    __client_db->setCallback(bind(&MQTTWorker::__updateDataCallback, this, _1));
    __update_timer->expires_from_now(boost::posix_time::seconds(__TIME_UPDATE));
    __update_timer->async_wait(boost::bind(&MQTTWorker::__updateTimerCallback, this, _1));

    __mqtt_broker->start(make_shared<shared_ptr<map<string, string>>>(__sp_db_map_login_password));
    __ms_worker->start(make_shared<shared_ptr<map<string, string>>>(__sp_db_map_lb_descriptor));
}
void MQTTWorker::__updateData(map<string, map<string, vector<string>>>& data) {
    shared_ptr<map<string, vector<string>>> temp_sp_db_lift_blocks = make_shared<map<string, vector<string>>>(data.at("LiftBlocks"));

    __sp_db_lift_blocks = temp_sp_db_lift_blocks;
    shared_ptr<map<string, string>> temp_sp_db_map_lb_descriptor = make_shared<map<string,string>>();
    shared_ptr<map<string, string>> temp_sp_db_map_login_password = make_shared<map<string, string>>();
    size_t position = 0;
    vector<size_t> vec_positions;
    string temp_id = "\"" + __id + "\"";
    for (auto i = __sp_db_lift_blocks->at("WorkerLuId").begin(),
        i_end = __sp_db_lift_blocks->at("WorkerLuId").end(),
        j = __sp_db_lift_blocks->at("WorkerLuSecId").begin(); i != i_end; i++,j++ ) 
    {
        if (temp_id == *i || temp_id == *j) {
            vec_positions.push_back(position);

        }
        position++;
    }
    if (vec_positions.size() == 0) {
        throw exception("Worker don't have LB");
    }
    for (auto i = vec_positions.begin(), end = vec_positions.end(); i != end; i++) {
        (*temp_sp_db_map_lb_descriptor)[__sp_db_lift_blocks->at("LiftId").at(*i)] = __sp_db_lift_blocks->at("Descriptor").at(*i);
        (*temp_sp_db_map_login_password)[__sp_db_lift_blocks->at("Login").at(*i)] = __sp_db_lift_blocks->at("Password").at(*i);
    }
    
    __sp_db_map_lb_descriptor = temp_sp_db_map_lb_descriptor;
    __sp_db_map_login_password  = temp_sp_db_map_login_password;
}
void MQTTWorker::__updateDataCallback(map<string, map<string, vector<string>>> data){
    try {
        __updateData(data);
    }
    catch (exception& e) {
        cerr << "__updateDataCallback: " << e.what();
        return;
    }
}
void MQTTWorker::__updateTimerCallback(const boost::system::error_code& error) {
    __client_db->setQuerys(__table_name, __table_fields);
    __client_db->start();
    __update_timer->expires_from_now(boost::posix_time::seconds(__TIME_UPDATE));
    __update_timer->async_wait(boost::bind(&MQTTWorker::__updateTimerCallback, this, _1));
}


int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");
    string config_file_name = "";
    for (int i = 1; i < argc; i++) {
        string flags = argv[i];
        if (flags == "-cf" || flags == "--config_file") {
            config_file_name = argv[++i];
        }
    }


    MQTTWorker worker(config_file_name);
    if (worker.init() != MQTTWorker::SUCCESSFUL) {
        cout << "Error reading Config file" << endl;
        return -1;
    }
    worker.start();
    return 0;

}