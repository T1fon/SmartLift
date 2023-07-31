#pragma once

#include "../Libraries/sqlite3.h"
#include "../../../GlobalModules/JSONFormatter/JSONFormatter.hpp"
#include "../../../GlobalModules/Log/Log.hpp"
#include "../../../GlobalModules/Config/Config.hpp"
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

#define DB_WAY "SmartLiftBase.db"

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;


class DataBase : public enable_shared_from_this<DataBase>
{
private:

	const string __Name = "Data_base";
	shared_ptr<tcp::endpoint>__endPoint;
	shared_ptr<tcp::socket>__socket;

	static const int BUF_SIZE = 2048;
	string __bufSend;
	char* __bufRecieve;
	boost::json::value __bufJsonRecieve;
	boost::json::stream_parser __parser;
	sqlite3* __dB;
	string __query;
	string __errorWhat;
	shared_ptr<Log> __log;
	bool __flagWrongConnect = false;

	void __reqAutentification();
	void __resAutentification(const boost::system::error_code& eC, size_t bytesSend);
	void __connectAnalize(const boost::system::error_code& eC, size_t bytesRecieve);
	void __waitCommand(const boost::system::error_code& eC, size_t bytesRecieve);
	void __sendCommand(const boost::system::error_code& eC, size_t bytesSend);

	string __checkCommand(char* __bufRecieve, size_t bytesRecieve);
	void __makePing();
	void __makeDisconnect();
	void __makeQuery();
	void __makeError();
	void __checkConnect(string login, string password);
	static int __connection(void* notUsed, int argc, char** argv, char** azColName);


public:
	DataBase(shared_ptr<Log> lg, tcp::socket sock);
	void start();
	void stop();
	~DataBase();
	shared_ptr<tcp::socket> getSocket();
};

class Server : public enable_shared_from_this<Server>
{
public:
	Server(shared_ptr<net::io_context> io_context, string nameConfigFile);
	void run();
	void stop();
private:
	shared_ptr<Log> __logServer;
	shared_ptr<Config> __config;
	map<string, string> __configInfo;
	static const int CONFIG_NUM_FIELDS = 1;
	vector<string> CONFIG_FIELDS = { "port" };
	shared_ptr<tcp::acceptor> __acceptor;
	shared_ptr<net::io_context> __ioc;
	std::shared_ptr<std::vector<std::shared_ptr<DataBase>>> __sessions;

	void __doAccept();
};