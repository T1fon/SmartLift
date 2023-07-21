#pragma once

#include "Modules/Libraries/sqlite3.h"
#include <boost/asio.hpp>
#include <boost/json.hpp>
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

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void fail(beast::error_code ErrCode, char const* what);

class DataBase : public enable_shared_from_this<DataBase>
{
private:
	const string __Name = "DataBse";
	string __id = "0";
	shared_ptr<tcp::endpoint>__endPoint;
	shared_ptr<tcp::socket>__socket;

	static const int BUF_SIZE = 2048;
	string __bufSend;
	char* __bufRecieve;
	boost::json::value __bufJsonRecieve;
	boost::json::stream_parser __parser;

	void __reqAutentification(const boost::system::error_code &eC);
	void __resAutentification(const boost::system::error_code& eC, size_t bytesSend);
	void __connectAnalize(const boost::system::error_code& eC, size_t bytesRecieve);

	void __waitCommand(const boost::system::error_code& eC, size_t bytesRecieve);
public:
	DataBase(string ip, string port, string idBase, net::io_context& ioc);
	void start();
	void stop();
	~DataBase();
	
};