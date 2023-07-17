#include "MSWorker.hpp"

MSWorker::MSWorker(string ip, string port, string id_worker, boost::asio::io_context& ioc) {
	__end_point = make_shared<net::tcp::endpoint>(net::tcp::endpoint(net::address::from_string(ip), stoi(port)));
	__socket = make_shared<net::tcp::socket>(net::tcp::socket(ioc));
}
void MSWorker::__authentication(const boost::system::error_code& error){
	
}
void MSWorker::start() {
	__socket->async_connect(*__end_point, boost::bind(&MSWorker::__authentication, shared_from_this(), _1));
}