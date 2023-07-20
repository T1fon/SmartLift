#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "JSONFormatter/JSONFormatter.hpp"

namespace net = boost::asio::ip;
//Main server client worker
class MSWorker: public std::enable_shared_from_this<MSWorker> {
private:
	const std::string __WORKER_NAME = "Worker_MQTT";
	std::string __id = "0";
	std::shared_ptr<net::tcp::endpoint> __end_point;
	std::shared_ptr<net::tcp::socket> __socket;

	static const int BUF_RECIVE_SIZE = 2048;
	std::string __buf_send;
	char *__buf_recive;
	boost::json::value __buf_json_recive;
	boost::json::stream_parser __parser;
	
	void __requestAuthentication(const boost::system::error_code &error);
	void __responseAuthentication(const boost::system::error_code &error, std::size_t count_send_byte);
	void __connectAnalize(const boost::system::error_code& error, std::size_t count_recive_byte);

	void __waitCommand(const boost::system::error_code& error, std::size_t count_recive_byte);
public:
	MSWorker(std::string ip, std::string port, std::string id_worker, boost::asio::io_context &ioc);
	void start();
	void stop();
	~MSWorker();
};