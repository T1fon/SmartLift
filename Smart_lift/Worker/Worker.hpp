/*#pragma once

#include "../GlobalModules/JSONFormatter/JSONFormatter.hpp"
#include <boost/lambda2.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <locale.h>
#include <ctime>
#include <map>
#include <list>
#include <queue>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Worker : public enable_shared_from_this<Worker>
{
private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	string __name = "Marusia_Worker";
	shared_ptr<tcp::endpoint>__endPoint;
	shared_ptr<tcp::socket>__socket;
	net::io_context& __ioc;

	static const int BUF_RECIVE_SIZE = 2048;
	string __bufSend;
	char* __bufRecive;
	boost::json::value __bufJsonRecive;
	boost::json::stream_parser __parser;

	map<string, string> __configInfo;

	void __connectToMS();
	void __recieveConnectToMS(const boost::system::error_code& eC, size_t bytesRecive);
	void __waitCommand(const boost::system::error_code& eC, size_t bytesRecive);
	void __connectToBd();
	void __recieveConnectToBd(const boost::system::error_code& eC, size_t bytesSend);
	void __makePing();


	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __checkSend(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);
	__CHECK_STATUS __checkJson(const size_t& countReciveByte, __handler_t&& handler);


public:
	Worker(shared_ptr<tcp::socket> socket, map<string, string> confInfo, net::io_context& ioc);//+Log
	~Worker();
	void start();
	void stop();
};*/