//(*end_p).publish("LU127728/set/cmd", std::string("{\"cmdlft\":[1,") + std::to_string(a.at(i)) + std::string("]}"));
#include "MQTTWorker.hpp"
MQTTWorker::MQTTWorker() {

}
MQTTWorker::~MQTTWorker() {
    this->stop();
}
void MQTTWorker::init() {
    MQTT_NS::setup_log();

    string port = DEFAULT_PORT;
	__mqtt_server = make_shared<MQTT_NS::server<>>(MQTT_NS::server<>(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            boost::lexical_cast<std::uint16_t>(port)
        ),
        __io_ctx
    ));
    __mqtt_broker.setServer(__mqtt_server);
    __mqtt_broker.init();
}
void MQTTWorker::start() {
    __mqtt_broker.start();

    //добавим позже потоки

    __io_ctx.run();
}
void MQTTWorker::stop() {
    __mqtt_broker.stop();
}