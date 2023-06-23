#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <boost/thread.hpp>
#include "Modules/MQTTBroker/MQTTBroker.hpp"

int main() {
    std::cout << "HIHIHIH" << std::endl;  
    MQTTBroker broker;
    MQTTBroker broker_2(2);
}
