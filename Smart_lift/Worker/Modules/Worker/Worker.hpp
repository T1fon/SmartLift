#pragma once
#include "../../../GlobalModules/Log/Log.hpp"
#include "../../../GlobalModules/Config/Config.hpp"
#include "../../../GlobalModules/JSONFormatter/JSONFormatter.hpp"
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
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <locale.h>
#include <ctime>
#include <map>
#include <list>
#include <queue>
#include <iostream>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class ClientDB : public enable_shared_from_this<ClientDB>
{
public:
	void __сallback(map<string, map<string, vector<string>>> data)
	{
		cerr << "ClientDb no Data to Worker" << endl;
	}
	typedef std::function<void(map<string, map<string, vector<string>>>)> _callback_t;
	_callback_t _callback;
	ClientDB(string ipBD, string portBD, shared_ptr<tcp::socket> socket, _callback_t callback) :
		_callback(boost::bind(&ClientDB::__сallback, this, boost::placeholders::_1))
	{
		_callback = callback;
		__endPoint = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ipBD), stoi(portBD)));
		__socket = socket;
		__bufRecive = new char[BUF_RECIVE_SIZE + 1];
		fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
		__bufJsonRecive = {};
		__parser.reset();
	}
	~ClientDB()
	{
		this->stop();
		delete[] __bufRecive;
	}
	void start()
	{
		__socket->async_connect(*__endPoint, boost::bind(&ClientDB::__checkConnect, shared_from_this(), boost::placeholders::_1));
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
	}


	void __checkConnect(const boost::system::error_code& eC)
	{
		if (eC)
		{
			cerr << eC.what() << endl;
			Sleep(2000);
			this->stop();
			this->start();
			return;
		}
		if (!__queueToSend.empty())
		{
			__bufQueueString = __queueToSend.front();
			__queueToSend.pop();
			__socket->async_send(boost::asio::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		else
		{
			_callback(__respData);
			this->stop();

		}
	}
	void __sendConnect(const boost::system::error_code& eC, size_t bytesSend)
	{
		if (eC)
		{
			cerr << "sendConnect" << eC.what() << endl;
			this->stop();
			this->start();
			return;
		}
		static size_t tempBytesSend = 0;
		/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
			return;
		}*/
		tempBytesSend += bytesSend;
		if (__bufQueueString.size() != tempBytesSend) {
			__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + tempBytesSend, (__bufQueueString.size() - tempBytesSend)),
				boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		tempBytesSend = 0;
		__bufQueueString.clear();
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __reciveCommand(const boost::system::error_code& eC, size_t bytesRecive)
	{
		if (eC)
		{
			cerr << "reciveCommand " << eC.what() << endl;
			this->stop();
			this->start();
			return;
		}
		/*if (__reciveCheck(bytesRecive, boost::bind(&Client::__reciveCommand, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL)
		{
			return;
		}*/
		try {
			__parser.write(__bufRecive, bytesRecive);
		}
		catch (exception& e) {
			cerr << e.what() << endl;
		}
		cout << __bufRecive << endl;
		fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);

		if (!__parser.done()) {
			cerr << "connectAnalize json not full" << endl;
			__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try {
			__parser.finish();
			__bufJsonRecive = __parser.release();
			__parser.reset();
		}
		catch (exception& e) {
			__parser.reset();
			__bufJsonRecive = {};
			cerr << "_reciveCheck " << e.what();
			return;
		}
		cout << __bufJsonRecive << endl;
		__commandAnalize(eC);
	}
	void __commandAnalize(const boost::system::error_code& eC)
	{
		boost::json::value target = __bufJsonRecive.at("target");
		boost::json::value status = __bufJsonRecive.at("response").at("status");
		__bufQueueString.clear();
		__bufQueueString = __bufJsonRecive.as_string();
		if (target == "connect")
		{
			{
				if (status != "success")
				{
					__respError.clear();
					__respError = __bufJsonRecive.as_string();
					this->stop();
					return;
				}
				else
				{
					__checkConnect(eC);

				}
			}
		}
		else if (target == "disconnect")
		{
			if (status == "success")
			{
				this->stop();
				return;
			}
			else
			{
				__respError.clear();
				__respError = __bufJsonRecive.as_string();
				__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
		}
		else if (target == "db_query")
		{
			map<string, vector<string>> bufRespMap;
			vector<string> valueMap;
			if (__bufJsonRecive.at("response").at(__marussiaStationFields[0]) != "")
			{
				for (size_t i = 0; i < __marussiaStationFields.size(); i++)
				{
					valueMap.clear();
					boost::json::array valueJson = __bufJsonRecive.at("response").at(__marussiaStationFields[i]).as_array();
					for (size_t j = 0; j < valueJson.size(); j++)
					{
						valueMap.push_back(boost::json::serialize(valueJson[j]));
					}
					bufRespMap[__marussiaStationFields[i]] = valueMap;
				}
				__respData["marussiaStation"] = bufRespMap;
			}
			else if (__bufJsonRecive.at("response").at(__houseFields[0]) != "")
			{
				for (size_t i = 0; i < __houseFields.size(); i++)
				{
					valueMap.clear();
					boost::json::array valueJson = __bufJsonRecive.at("response").at(__houseFields[i]).as_array();
					for (size_t j = 0; j < valueJson.size(); j++)
					{
						valueMap.push_back(boost::json::serialize(valueJson[j]));
					}
					bufRespMap[__houseFields[i]] = valueMap;
				}
				__respData["houseFields"] = bufRespMap;
			}
			else if (__bufJsonRecive.at("response").at(__staticPhrasesFields[0]) != "")
			{
				for (size_t i = 0; i < __staticPhrasesFields.size(); i++)
				{
					valueMap.clear();
					boost::json::array valueJson = __bufJsonRecive.at("response").at(__staticPhrasesFields[i]).as_array();
					for (size_t j = 0; j < valueJson.size(); j++)
					{
						valueMap.push_back(boost::json::serialize(valueJson[j]));
					}
					bufRespMap[__staticPhrasesFields[i]] = valueMap;
				}
				__respData["staticPhrases"] = bufRespMap;
			}
			__checkConnect(eC);
		}
	}
	void setQuery(queue<string> queue)
	{
		__queueToSend = queue;
	}
	map<string, map<string, vector<string>>> getRespData()
	{
		return __respData;
	}

private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	shared_ptr<tcp::endpoint> __endPoint;
	shared_ptr<tcp::socket> __socket;
	static const int BUF_RECIVE_SIZE = 2048;
	queue<string> __queueToSend;
	string __bufQueueString;
	char* __bufRecive;
	boost::json::value __bufJsonRecive;
	boost::json::stream_parser __parser;
	string __respError;
	map<string, map<string, vector<string>>> __respData;
	vector<string> __marussiaStationFields = { "ApplicationId", "ComplexId", "HouseNum" };
	vector<string> __houseFields = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> __staticPhrasesFields = { "ComplexId", "HouseNumber", "KeyWords", "Response" };

	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __reciveCheck(const size_t& count_recive_byte, __handler_t&& handler);
	__CHECK_STATUS __sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);

};

class ClientMS : public enable_shared_from_this<ClientMS>
{
public:
	ClientMS(string ipMS, string portMS, shared_ptr<tcp::socket> socket)
	{
		__endPoint = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ipMS), stoi(portMS)));
		__socket = socket;
		__bufRecive = new char[BUF_RECIVE_SIZE + 1];
		fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
		__bufJsonRecive = {};
		__parser.reset();
	}
	~ClientMS()
	{
		this->stop();
		delete[] __bufRecive;
	}
	void start()
	{
		__socket->async_connect(*__endPoint, boost::bind(&ClientMS::__readFromW, shared_from_this(), boost::placeholders::_1));
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
	}

private:
	void __readFromW(const boost::system::error_code& eC)
	{
		if (eC)
		{
			cerr << eC.what() << endl;
			Sleep(2000);
			this->stop();
			this->start();
			return;
		}
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendFromWTMS, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __sendFromWTMS(const boost::system::error_code& eC, size_t bytesRecive)
	{
		if (eC)
		{
			cerr << "reciveCommand " << eC.what() << endl;
			this->stop();
			this->start();
			return;
		}
		/*if (__reciveCheck(bytesRecive, boost::bind(&Client::__reciveCommand, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL)
		{
			return;
		}*/
		try {
			__parser.write(__bufRecive, bytesRecive);
		}
		catch (exception& e) {
			cerr << e.what() << endl;
		}
		cout << __bufRecive << endl;
		fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);

		if (!__parser.done()) {
			cerr << "connectAnalize json not full" << endl;
			__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendFromWTMS, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try {
			__parser.finish();
			__bufJsonRecive = __parser.release();
			__parser.reset();
		}
		catch (exception& e) {
			__parser.reset();
			__bufJsonRecive = {};
			cerr << "_reciveCheck " << e.what();
			return;
		}
		__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientMS::__resFromMStW, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __resFromMStW(const boost::system::error_code& eC, size_t bytesSend)
	{
		if (eC)
		{
			cerr << "sendConnect" << eC.what() << endl;
			this->stop();
			this->start();
			return;
		}
		static size_t tempBytesSend = 0;
		/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
			return;
		}*/
		tempBytesSend += bytesSend;
		if (__bufQueueString.size() != tempBytesSend) {
			__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + tempBytesSend, (__bufQueueString.size() - tempBytesSend)),
				boost::bind(&ClientMS::__resFromMStW, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		tempBytesSend = 0;
		__bufQueueString.clear();
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendResTW, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __sendResTW(const boost::system::error_code& eC, size_t bytesRecive)
	{
		if (eC)
		{
			cerr << "reciveCommand " << eC.what() << endl;
			this->stop();
			this->start();
			return;
		}
		/*if (__reciveCheck(bytesRecive, boost::bind(&Client::__reciveCommand, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL)
		{
			return;
		}*/
		try {
			__parser.write(__bufRecive, bytesRecive);
		}
		catch (exception& e) {
			cerr << e.what() << endl;
		}
		cout << __bufRecive << endl;
		fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);

		if (!__parser.done()) {
			cerr << "connectAnalize json not full" << endl;
			__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendResTW, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try {
			__parser.finish();
			__bufJsonRecive = __parser.release();
			__parser.reset();
		}
		catch (exception& e) {
			__parser.reset();
			__bufJsonRecive = {};
			cerr << "_reciveCheck " << e.what();
			return;
		}
		__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientMS::__readFromW, shared_from_this(), boost::placeholders::_1));
	}

	shared_ptr<tcp::endpoint> __endPoint;
	shared_ptr<tcp::socket> __socket;
	static const int BUF_RECIVE_SIZE = 2048;
	queue<string> __queueToSend;
	string __bufQueueString;
	char* __bufRecive;
	boost::json::value __bufJsonRecive;
	boost::json::stream_parser __parser;
};

class Worker : public enable_shared_from_this<Worker>
{
public:
	typedef std::function<void(map<string, map<string, vector<string>>> )> _callback_t;
	_callback_t callback;

	void __сallback(map<string, map<string, vector<string>>> data)
	{
		setDbInfo(data);
	}

	Worker(map<string, string> confInfo, net::io_context& ioc, shared_ptr<Log> lg) :__ioc(ioc),
		callback(boost::bind(&Worker::__сallback, this, boost::placeholders::_1))
	{
		cout << 1 << endl;
		__socket = make_shared<tcp::socket>(__ioc);
		__bufRecive = new char[BUF_RECIVE_SIZE + 1];
		__bufSend = "";
		__bufJsonRecive = {};
		__parser.reset();
		__configInfo = confInfo;
		__timer = make_shared<net::deadline_timer>(__ioc);
		__log = lg;
		try
		{
			__ipMS = __configInfo.at("Main_server_ip");
			__portMS = __configInfo.at("Main_server_port");
			__workerId = __configInfo.at("Id");
			__ipBd = __configInfo.at("BD_ip");
			__portBd = __configInfo.at("BD_port");
			__bDLog = __configInfo.at("BD_login");
			__bDPas = __configInfo.at("BD_password");
			__socketDB = make_shared<tcp::socket>(__ioc);
			__dBClient = make_shared<ClientDB>(__ipBd, __portBd, __socketDB, callback);
			__mSClient = make_shared<ClientMS>(__ipMS, __portMS, __socket);


		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
	}
	~Worker()
	{
		this->stop();
		delete[] __bufRecive;
	}
	void start()
	{
		cout << 2 << endl;
		__log->writeLog(0, __name, "start_server");
		__log->writeTempLog(0, __name, "start_server");
		__timer->expires_from_now(boost::posix_time::hours(24));
		__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
		__mSClient->start();
		__connectToBd();
		__connectToMS();
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
	}

	void setDbInfo(map <string, map<string, vector<string>>> data)
	{
		__dBInfo = data;

	}

private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	string __name = "Marusia_Worker";
	shared_ptr<tcp::socket>__socket;
	shared_ptr<tcp::socket> __socketDB;
	net::io_context& __ioc;
	shared_ptr<ClientMS> __mSClient;
	shared_ptr<ClientDB> __dBClient;
	shared_ptr<net::deadline_timer> __timer;

	string __id;
	string __ipMS;
	string __portMS;
	string __workerId;
	string __ipBd;
	string __portBd;
	string __bDLog;
	string __bDPas;

	map<string, map<string, vector<string>>> __dBInfo;
	vector<string> __marussiaStationFields = { "ApplicationId", "ComplexId", "HouseNum" };
	vector<string> __houseFields = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> __staticPhrasesFields = { "ComplexId", "HouseNumber", "KeyWords", "Response" };


	static const int BUF_RECIVE_SIZE = 2048;
	string __bufSend;
	char* __bufRecive;
	boost::json::value __bufJsonRecive;
	boost::json::value __bufJsonResponse;
	boost::json::stream_parser __parser;
	queue<string> __sender;
	shared_ptr<Log> __log;

	vector<string> __keyRoots = { "перв", "втор", "трет", "чет", "пят", "шест", "седьм", "восьм", "девят","дцат", "сорок", "десят",  "девян", "сот","сто","минус" };
	map<string, int> __numRoots = {
		{"минус третий", -3}, {"минус второй", -2}, {"минус первый", -1}, {"нулевой", 0},
		{"первый", 1}, {"второй", 2}, {"третий", 3}, {"четвертый", 4}, {"пятый", 5}, {"шестой", 6}, {"седьмой", 7}, {"восьмой", 8},{"девятый", 9}, {"десятый", 10},
		{"одиннадцатый", 11}, {"двенадцатый", 12}, {"тринадцатый", 13}, {"четырнадцатый", 14}, {"пятнадцатый", 15}, {"шестнадцатый", 16}, {"семнадцатый", 17}, {"восемнадцатый", 18},
		{"девятнадцатый", 19}, {"двадцатый", 20}, {"двадцать первый", 21}, {"двадцать второй", 22}, {"двадцать третий", 23}, {"двадцать четвертый", 24}, {"двадцать пятый", 25},
		{"двадцать шестой", 26}, {"двадцать седьмой", 27}, {"двадцать восьмой", 28}, {"двадцать девятый", 29}, {"тридцатый", 30}
	};
	map<string, string> __configInfo;

	void __connectToMS()
	{
		cout << 3 << endl;
		__log->writeTempLog(0, __name, "connect_to_server");
		__bufSend = boost::json::serialize(json_formatter::worker::request::connect(__name, __id));
		__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
			boost::placeholders::_1, boost::placeholders::_2));
	}
	void __recieveConnectToMS(const boost::system::error_code& eC, size_t bytesSend)
	{
		if (eC)
		{
			cerr << "Send " << eC.what() << endl;
			__log->writeLog(3, __name, "connect_to_ms");
			__log->writeTempLog(3, __name, "connect_to_ms");
			Sleep(1000);
			__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		static size_t tempBytesSend = 0;
		/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
			return;
		}*/
		tempBytesSend += bytesSend;
		if (__bufSend.size() != tempBytesSend) {
			__log->writeTempLog(0, __name, "not_full_send");
			__socket->async_send(boost::asio::buffer(__bufSend.c_str() + tempBytesSend, (__bufSend.size() - tempBytesSend)),
				boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		tempBytesSend = 0;
		__log->writeTempLog(0, __name, "recieve to_ms");
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __waitCommand(const boost::system::error_code& eC, size_t bytesRecive)
	{
		if (eC)
		{
			cerr << "waitCommand " << eC.what() << endl;
			__log->writeLog(0, __name, "no_data_from_ms");
			__log->writeTempLog(0, __name, "no_data_from_ms");
			Sleep(1000);
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		try
		{
			__parser.write(__bufRecive, bytesRecive);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
		fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
		if (!__parser.done())
		{
			__log->writeTempLog(0, __name, "not_full_json");
			cerr << "Connect analize json not full" << endl;
			__log->writeTempLog(2, "DataBase", "Not_full_json");
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try
		{
			__parser.finish();
			__bufJsonRecive = __parser.release();
			__parser.reset();
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			__log->writeTempLog(0, __name, "error_writting");
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		boost::json::value target = __bufJsonRecive.at("target");
		boost::json::value status = __bufJsonRecive.at("response").at("status");

		if (target == "ping")
		{
			__log->writeTempLog(0, __name, "ping");
			string ping = boost::json::serialize(json_formatter::worker::response::ping(__name));
			__bufSend = boost::json::serialize(json_formatter::worker::response::ping(__name));
		}
		else if (target == "disconnect")
		{
			__log->writeTempLog(0, __name, "disconnect");
			string respDisconnect = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
			__bufSend = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
		}
		else if (target == "marussia_station_request")
		{
			__log->writeTempLog(0, __name, "analize_response");
			__analizeRequest();
		}
		else if (target == "connect")
		{

			if (status == "success")
			{
				__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
					boost::placeholders::_1, boost::placeholders::_2));
			}
			else if (status == "fail")
			{
				__log->writeLog(3, __name, "error_connect");
				boost::json::value fail = __bufJsonRecive.at("response").at("message");
				cerr << "error connect " << fail << endl;
				__connectToMS();
			}
		}
		else
		{
			__log->writeLog(5, __name, "no_command");
			__log->writeTempLog(5, __name, "no_command");
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
		}

		__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
			boost::placeholders::_1, boost::placeholders::_2));
	}
	void __connectToBd()
	{
		string Connect = boost::json::serialize(json_formatter::database::request::connect(__name, __bDLog, __bDPas));
		string selectMarussiaStation = boost::json::serialize(json_formatter::database::request::query(__name,
			json_formatter::database::QUERY_METHOD::SELECT, __marussiaStationFields, "SELECT * FROM MarussiaStation"));
		string selectHouse = boost::json::serialize(json_formatter::database::request::query(__name,
			json_formatter::database::QUERY_METHOD::SELECT, __houseFields, "SELECT * FROM House"));
		string selectStaticPhrases = boost::json::serialize(json_formatter::database::request::query(__name,
			json_formatter::database::QUERY_METHOD::SELECT, __staticPhrasesFields, "SELECT * FROM StaticPhrases"));
		string disconnect = boost::json::serialize(json_formatter::database::request::disconnect(__name));
		queue<string> requestBody;
		requestBody.push(Connect);
		requestBody.push(selectMarussiaStation);
		requestBody.push(selectHouse);
		requestBody.push(selectStaticPhrases);
		requestBody.push(disconnect);

		__dBClient->setQuery(requestBody);
		__dBClient->start();
		__waitDbInfo();
	}

	void __waitDbInfo()
	{
		if (__dBInfo.empty())
		{
			__waitDbInfo();
		}
		else
		{
			__dBClient->stop();
		}

	}
	void __resetTimer()
	{
		__log->writeTempLog(0, __name, "__rewrite_data_base");
		__connectToBd();
		__timer->expires_from_now(boost::posix_time::hours(24));
		__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
	}

	void __analizeRequest()
	{
		__log->writeTempLog(0, __name, "__analize_response");
		///___marussia station House number and complex id___///
		string appId = boost::json::serialize(__bufJsonRecive.at("application").at("application_id"));
		map<string, vector<string>> oneTable = __dBInfo.at("MarussiaStation");
		vector<string> bufVec = __dBInfo.at("MarussiaStation").at("ApplicationId");
		vector<string> houseVec = __dBInfo.at("MarussiaStation").at("HouseNum");
		vector<string> compVec = __dBInfo.at("MarussiaStation").at("ComplexId");
		vector<string> liftVec = __dBInfo.at("MarussiaStation").at("LiftBlockId");

		string numHouse = "-1";
		string compId = "-1";
		string liftblock = "";
		string response = "";

		for (size_t i = 0; i < bufVec.size(); i++)
		{
			if (appId == bufVec[i])
			{
				numHouse = houseVec[i];
				compId = compVec[i];
				liftblock = liftVec[i];
				break;
			}
		}

		///___search for mqtt_____/////
		boost::json::array ar = __bufJsonRecive.at("request").at("nlu").at("tokens").as_array();
		vector<string> searchMqtt;
		bool flagMqtt = false;
		for (size_t i = 0; i < ar.size(); i++)
		{
			searchMqtt.push_back(boost::json::serialize(ar[i]));
			if (ar[i].as_string() == "этаж" || ar[i].as_string() == "подъем" || ar[i].as_string() == "спуск" || ar[i].as_string() == "подними" || ar[i].as_string() == "опусти")
			{
				flagMqtt = true;
			}
		}
		if (flagMqtt)
		{
			oneTable.clear();
			oneTable = __dBInfo.at("House");
			bufVec.clear(); houseVec.clear(); compVec.clear();
			houseVec = __dBInfo.at("House").at("HouseNum");
			compVec = __dBInfo.at("House").at("ComplexId");
			vector<string> topFloor = __dBInfo.at("House").at("TopFloor");
			vector<string> botFl = __dBInfo.at("House").at("BottomFloor");
			vector<string> nullFl = __dBInfo.at("House").at("NullFloor");
			string bufNum = "";
			int floor;
			for (size_t i = 0; i < __keyRoots.size(); i++)
			{
				for (size_t j = 0; j < searchMqtt.size(); j++)
				{
					if (searchMqtt[j].find(__keyRoots[i]) != searchMqtt[j].npos)
					{
						bufNum += searchMqtt[j];
					}
				}
			}
			int numFloor = __numRoots.at(bufNum);

			for (size_t i = 0; i < houseVec.size(); i++)
			{
				if (houseVec[i] == numHouse && compVec[i] == compId)
				{
					if (topFloor[i] == to_string(numFloor) || botFl[i] == to_string(numFloor) || nullFl[i] == to_string(numFloor))
					{
						response = "перемещаю вас на " + bufNum + "этаж";
						break;
					}
				}
			}
			__bufSend = boost::json::serialize(json_formatter::worker::response::marussia_mqtt_message(__name, appId, __getRespToMS(response), liftblock, numFloor));
		}
		else
		{
			///_____static phrases response static phrase___///

			string command = boost::json::serialize(__bufJsonRecive.at("request").at("command"));
			oneTable.clear();
			oneTable = __dBInfo.at("StaticPhrases");
			bufVec.clear(); houseVec.clear(); compVec.clear();
			bufVec = __dBInfo.at("StaticPhrases").at("KeyWords");
			houseVec = __dBInfo.at("StaticPhrases").at("HouseNumber");
			compVec = __dBInfo.at("StaticPhrases").at("ComplexId");

			vector<string> resp = __dBInfo.at("StaticPhrases").at("Response");

			response.clear();

			for (size_t i = 0; i < bufVec.size(); i++)
			{
				if (command == bufVec[i])
				{
					if (numHouse == houseVec[i] && compId == compVec[i])
					{
						response = resp[i];
						break;
					}
				}
			}
			if (response != "")
			{
				__bufSend = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, appId, __getRespToMS(response)));
			}
			else
			{
				response = "Извините, я не знаю такой комманды, попробуйте перефразировать";
				__bufSend = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, appId, __getRespToMS(response)));
			}
		}

	}
	boost::json::object __getRespToMS(string respText)
	{
		return boost::json::object({ {"response",{
											{"text", respText},
											{"tts", respText},
											{"end_session", "false"}
										}} });
	}


	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __checkSend(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);
	__CHECK_STATUS __checkJson(const size_t& countReciveByte, __handler_t&& handler);

};
//-------------------------------------------------------------//


class Server : public enable_shared_from_this<Server>
{
public:
	Server(shared_ptr<net::io_context> io_context, string nameConfigFile)
	{
		__ioc = io_context;
		__sessions = make_shared<std::vector<std::shared_ptr<Worker>>>();
		__logServer = make_shared<Log>("Log/", "./", "Marussia_Worker");
		__config = make_shared<Config>(__logServer, "./", "", nameConfigFile);
		__config->readConfig();
		__configInfo = __config->getConfigInfo();
		if (__configInfo.size() == 0)
		{
			__logServer->writeLog(1, "Marussia_Worker", "Config_File_not_open");
			return;
		}
		else if (__configInfo.size() < CONFIG_NUM_FIELDS)
		{
			__logServer->writeLog(1, "Marussia_Worker", "Config_File_not_full");
		}
		try
		{
			for (size_t i = 0, length = __configInfo.size(); i < length; i++)
			{
				__configInfo.at(CONFIG_FIELDS.at(i));
			}
		}
		catch (exception& e)
		{
			__logServer->writeLog(1, "Marussia_Worker", e.what());
			return;
		}
		string ip = __configInfo.at("Main_server_ip");
		string port = __configInfo.at("Main_server_port");
		cerr << port << endl;
		__acceptor = make_shared<tcp::acceptor>(*__ioc, tcp::endpoint(net::ip::make_address(ip), stoi(port)));

	}
	void run()
	{
		__doAccept();
	}
	void stop()
	{
		for (size_t i = 0, length = __sessions->size(); i < length; i++) {
			__sessions->back()->stop();
		}
		__sessions->clear();
	}
	~Server()
	{
		this->stop();
	}
private:
	shared_ptr<Log> __logServer;
	shared_ptr<Config> __config;
	map<string, string> __configInfo;
	shared_ptr<Worker> __worker;
	shared_ptr<tcp::socket> __sock;
	static const int CONFIG_NUM_FIELDS = 1;
	vector<string> CONFIG_FIELDS = { "Id", "Main_server_ip", "Main_server_port", "BD_ip", "BD_port", "BD_login", "BD_password"};
	shared_ptr<tcp::acceptor> __acceptor;
	shared_ptr<net::io_context> __ioc;
	std::shared_ptr<std::vector<std::shared_ptr<Worker>>> __sessions;

	void __doAccept()
	{
		cerr << "01" << endl;
		cerr << "hi" << endl;
		__worker = make_shared<Worker>(__configInfo, *__ioc, __logServer);
		__worker->start();
	}
};


