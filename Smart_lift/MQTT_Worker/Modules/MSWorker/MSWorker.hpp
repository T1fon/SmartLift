#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "JSONFormatter/JSONFormatter.hpp"

namespace net = boost::asio::ip;
//Main server client worker
class MSWorker: public std::enable_shared_from_this<MSWorker> {
public:
	typedef std::function<void(std::string, std::string, std::string)> callback_mqtt_worker_t;
	
private:

	enum __CHECK_STATUS {
		SUCCESS = 1,
		FAIL
	};

	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	
	const std::string __WORKER_NAME = "Worker_MQTT";
	std::string __id = "0";
	std::shared_ptr<net::tcp::endpoint> __end_point;
	std::shared_ptr<net::tcp::socket> __socket;
	std::vector<std::string>& __lu_id;

	static const int BUF_RECIVE_SIZE = 2048;
	std::string __buf_send;
	char *__buf_recive;
	boost::json::value __buf_json_recive;
	boost::json::stream_parser __parser;
	callback_mqtt_worker_t __callback_mqtt_worker;
	
	void __requestAuthentication(const boost::system::error_code &error);
	void __connectAnalize();
	void __responsePing();
	void __responseDisconnect();

	void __sendCommand(const boost::system::error_code& error, std::size_t count_send_byte);
	void __reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte);
	
	void __commandAnalize();
	void __moveLift();
	void __emptyCallback(std::string lu_id, std::string floor_number, std::string station_id);
	
	__CHECK_STATUS __reciveCheck(const size_t &count_recive_byte,  __handler_t &&handler);
	__CHECK_STATUS __sendCheck(const size_t &count_send_byte,size_t &temp_send_byte, __handler_t &&handler);
public:

	MSWorker(std::string ip, std::string port, std::string id_worker, std::vector<std::string>& lu_id, boost::asio::io_context &ioc);
	void start();
	void stop();
	void setCallback(callback_mqtt_worker_t callback_mqtt_worker);
	void successMove(std::string station_id);
	~MSWorker();
};