#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;
namespace net = boost::asio::ip;
//Main server client worker
class MSWorker: public enable_shared_from_this<MSWorker> {
private:
	shared_ptr<net::tcp::endpoint> __end_point;
	shared_ptr<net::tcp::socket> __socket;

	void __authentication(const boost::system::error_code &error);
	//void __authentication();
public:
	MSWorker(string ip, string port,string id_worker, boost::asio::io_context &ioc);
	void start();
	~MSWorker();
};