//(*end_p).publish("LU127728/set/cmd", std::string("{\"cmdlft\":[1,") + std::to_string(a.at(i)) + std::string("]}"));
#include "MQTTWorker.hpp"
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
    
    /*заполнение __lu_id*/

    /*------------------*/

    MQTT_NS::setup_log();

	__mqtt_server = make_shared<MQTT_NS::server<>>(MQTT_NS::server<>(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            boost::lexical_cast<std::uint16_t>(__main_server_port)
        ),
        __io_ctx
    ));
    __mqtt_broker = make_shared<mqtt_broker::MQTTBroker>();
    __mqtt_broker->setServer(__mqtt_server);
    __mqtt_broker->init();
    __ms_worker = make_shared<MSWorker>(__main_server_ip, to_string(__main_server_info_port), __id,__lu_ids, __io_ctx);
    __ms_worker->setCallback(bind(&MQTTWorker::moveLift, this, _1, _2, _3));
    __mqtt_worker = __mqtt_broker->getWorker();
    return SUCCESSFUL;
}
void MQTTWorker::start() {
    __mqtt_broker->start();
    __ms_worker->start();

    std::vector<std::thread> v;
    v.reserve(__count_threads - 1);
    for (auto i = __count_threads - 1; i > 0; --i)
        v.emplace_back(
            [this]
            {
                __io_ctx.run();
            });
    __io_ctx.run();
}
void MQTTWorker::stop() {
    __mqtt_broker->stop();
    __ms_worker->stop();
}

void MQTTWorker::moveLift(string lu_id, string floor_number, string station_id) {
    __mqtt_worker->async_publish("LU" + lu_id +"/set/cmd", "{\"cmdlft\":[1," + floor_number + "]}");
    __ms_worker->successMove(station_id);
}