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

    __logger = make_shared<Log>("", "test_log.log");
    __configer = make_shared<Config>(__logger,"./","");
    
    __configer->readConfig();
    __config_data = __configer->getConfigInfo();

    if (__config_data.size() == 0) {
        return CONFIG_FILE_NOT_OPEN;
    }
    else if (__config_data.size() < __COUNT_CONFIG_FIELDS) {
        return CONFIG_DATA_NOT_FULL;
    }

    try {
        for (int i = 0, length = __CONFIG_FIELDS.size(); i < length; i++) {
            __config_data.at(__CONFIG_FIELDS.at(i));
        }
    }
    catch (exception& e) {
        cout << e.what() << endl;
        return CONFIG_DATA_NOT_FULL;
    }

    MQTT_NS::setup_log();

    string port = DEFAULT_PORT;
	__mqtt_server = make_shared<MQTT_NS::server<>>(MQTT_NS::server<>(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            boost::lexical_cast<std::uint16_t>(port)
        ),
        __io_ctx
    ));
    __mqtt_broker = make_shared<mqtt_broker::MQTTBroker>();
    __mqtt_broker->setServer(__mqtt_server);
    __mqtt_broker->init();
    __ms_worker = make_shared<MSWorker>(__config_data["Main_server_ip"], __config_data["Main_server_info_port"], __config_data["Id"], __io_ctx);
    return SUCCESSFUL;
}
void MQTTWorker::start() {
    //__mqtt_broker.start();
    
    
    __ms_worker->start();

    //добавим позже потоки

    __io_ctx.run();
}
void MQTTWorker::stop() {
    //__mqtt_broker.stop();
}