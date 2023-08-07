#pragma once
#include "../../../GlobalModules/Log/Log.hpp"
#include "../../../GlobalModules/Config/Config.hpp"
#include "../../../GlobalModules/JSONFormatter/JSONFormatter.hpp"
#include "../../../GlobalModules/ClientDB/ClientDB.hpp"
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

	Worker(map<string, string> conf_info, net::io_context& ioc, shared_ptr<Log> lg) :__ioc(ioc),
		callback(boost::bind(&Worker::__сallback, this, boost::placeholders::_1))
	{
		cout << 1 << endl;
		__socket = make_shared<tcp::socket>(__ioc);
		__buf_recive = new char[BUF_RECIVE_SIZE + 1];
		__buf_send = "";
		__buf_json_recive = {};
		__parser.reset();
		__config_info = conf_info;
		__timer = make_shared<net::deadline_timer>(__ioc);
		__log = lg;
		try
		{
			__ip_ms = __config_info.at("Main_server_ip");
			__port_ms = __config_info.at("Main_server_port");
			__worker_id = __config_info.at("Id");
			__ip_db = __config_info.at("BD_ip");
			__port_db = __config_info.at("BD_port");
			__db_log = __config_info.at("BD_login");
			__db_pas = __config_info.at("BD_password");
			__socket_dB = make_shared<tcp::socket>(__ioc);
			__db_client = make_shared<ClientDB>(__ip_db, __port_db, __db_log, __db_pas, __name, __socket_dB, callback);
			__ms_client = make_shared<ClientMS>(__ip_ms, __port_ms, __socket);


		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			this->stop();
			return;
		}
	}
	~Worker()
	{
		this->stop();
		delete[] __buf_recive;
	}
	void start()
	{
		cout << 2 << endl;
		__log->writeLog(0, __name, "start_server");
		__log->writeTempLog(0, __name, "start_server");
		__timer->expires_from_now(boost::posix_time::hours(24));
		__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
		__ms_client->start();
		__connectToBd();
		//__connectToMS();
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
		__db_info = data;

	}

private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	string __name = "Marusia_Worker";
	shared_ptr<tcp::socket>__socket;
	shared_ptr<tcp::socket> __socket_dB;
	net::io_context& __ioc;
	shared_ptr<ClientMS> __ms_client;
	shared_ptr<ClientDB> __db_client;
	shared_ptr<net::deadline_timer> __timer;

	string __id;
	string __ip_ms;
	string __port_ms;
	string __worker_id;
	string __ip_db;
	string __port_db;
	string __db_log;
	string __db_pas;

	map<string, map<string, vector<string>>> __db_info;
	/*vector<string> __marussiaStationFields = {"ApplicationId", "ComplexId", "HouseNum"};
	vector<string> __houseFields = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> __staticPhrasesFields = { "ComplexId", "HouseNumber", "KeyWords", "Response" };*/


	static const int BUF_RECIVE_SIZE = 2048;
	string __buf_send;
	char* __buf_recive;
	boost::json::value __buf_json_recive;
	boost::json::value __buf_json_response;
	boost::json::stream_parser __parser;
	queue<string> __sender;
	shared_ptr<Log> __log;

	vector<string> __key_roots = { "перв", "втор", "трет", "чет", "пят", "шест", "седьм", "восьм", "девят","дцат", "сорок", "десят",  "девян", "сот","сто","минус" };
	map<string, int> __num_roots = {
		{"минус третий", -3}, {"минус второй", -2}, {"минус первый", -1}, {"нулевой", 0},
		{"первый", 1}, {"второй", 2}, {"третий", 3}, {"четвертый", 4}, {"пятый", 5}, {"шестой", 6}, {"седьмой", 7}, {"восьмой", 8},{"девятый", 9}, {"десятый", 10},
		{"одиннадцатый", 11}, {"двенадцатый", 12}, {"тринадцатый", 13}, {"четырнадцатый", 14}, {"пятнадцатый", 15}, {"шестнадцатый", 16}, {"семнадцатый", 17}, {"восемнадцатый", 18},
		{"девятнадцатый", 19}, {"двадцатый", 20}, {"двадцать первый", 21}, {"двадцать второй", 22}, {"двадцать третий", 23}, {"двадцать четвертый", 24}, {"двадцать пятый", 25},
		{"двадцать шестой", 26}, {"двадцать седьмой", 27}, {"двадцать восьмой", 28}, {"двадцать девятый", 29}, {"тридцатый", 30}
	};
	map<string, string> __config_info;

	void __connectToMS()
	{
		cout << 3 << endl;
		__log->writeTempLog(0, __name, "connect_to_server");
		__buf_send = boost::json::serialize(json_formatter::worker::request::connect(__name, __id));
		__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
			boost::placeholders::_1, boost::placeholders::_2));
	}
	void __recieveConnectToMS(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			cerr << "Send " << eC.what() << endl;
			__log->writeLog(3, __name, "connect_to_ms");
			__log->writeTempLog(3, __name, "connect_to_ms");
			Sleep(1000);
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		static size_t temp_bytes_send = 0;
		/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
			return;
		}*/
		temp_bytes_send += bytes_send;
		if (__buf_send.size() != temp_bytes_send) {
			__log->writeTempLog(0, __name, "not_full_send");
			__socket->async_send(boost::asio::buffer(__buf_send.c_str() + temp_bytes_send, (__buf_send.size() - temp_bytes_send)),
				boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_bytes_send = 0;
		__log->writeTempLog(0, __name, "recieve to_ms");
		__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __waitCommand(const boost::system::error_code& eC, size_t bytes_recive)
	{
		if (eC)
		{
			cerr << "waitCommand " << eC.what() << endl;
			__log->writeLog(0, __name, "no_data_from_ms");
			__log->writeTempLog(0, __name, "no_data_from_ms");
			Sleep(1000);
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		try
		{
			__parser.write(__buf_recive, bytes_recive);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
		fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
		if (!__parser.done())
		{
			__log->writeTempLog(0, __name, "not_full_json");
			cerr << "Connect analize json not full" << endl;
			__log->writeTempLog(2, "DataBase", "Not_full_json");
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try
		{
			__parser.finish();
			__buf_json_recive = __parser.release();
			__parser.reset();
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			__log->writeTempLog(0, __name, "error_writting");
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		boost::json::value target = __buf_json_recive.at("target");
		boost::json::value status = __buf_json_recive.at("response").at("status");

		if (target == "ping")
		{
			__log->writeTempLog(0, __name, "ping");
			string ping = boost::json::serialize(json_formatter::worker::response::ping(__name));
			__buf_send = boost::json::serialize(json_formatter::worker::response::ping(__name));
		}
		else if (target == "disconnect")
		{
			__log->writeTempLog(0, __name, "disconnect");
			string respDisconnect = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
			__buf_send = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
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
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
					boost::placeholders::_1, boost::placeholders::_2));
			}
			else if (status == "fail")
			{
				__log->writeLog(3, __name, "error_connect");
				boost::json::value fail = __buf_json_recive.at("response").at("message");
				cerr << "error connect " << fail << endl;
				this->stop();
				return;
			}
		}
		else
		{
			__log->writeLog(5, __name, "no_command");
			__log->writeTempLog(5, __name, "no_command");
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
		}

		__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
			boost::placeholders::_1, boost::placeholders::_2));
	}
	void __connectToBd()
	{

		queue<string> tables;
		tables.push("MarussiaStation"); tables.push("House"); tables.push("StaticPhrases");
		queue<vector<string>> fields;
		vector<string> fields_marussia = { "ApplicationId", "ComplexId", "HouseNum" };
		vector<string> fields_house = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
		vector<string> fields_phrases = { "ComplexId", "HouseNumber", "KeyWords", "Response" };
		fields.push(fields_marussia); fields.push(fields_house); fields.push(fields_phrases);

		cout << __db_log << " " << __db_pas << endl;
		cout << __ip_db << " " << __port_db << endl;
		__db_client->setQuerys(tables, fields);
		__db_client->start();
		//__waitDbInfo();
	}

	void __waitDbInfo()
	{
		if (__db_info.empty())
		{
			__waitDbInfo();
		}
		else
		{
			__db_client->stop();
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
		string app_id = boost::json::serialize(__buf_json_recive.at("application").at("application_id"));
		map<string, vector<string>> one_table = __db_info.at("MarussiaStation");
		vector<string> buf_vec = __db_info.at("MarussiaStation").at("ApplicationId");
		vector<string> house_vec = __db_info.at("MarussiaStation").at("HouseNum");
		vector<string> comp_vec = __db_info.at("MarussiaStation").at("ComplexId");
		vector<string> lift_vec = __db_info.at("MarussiaStation").at("LiftBlockId");

		string num_house = "-1";
		string comp_id = "-1";
		string lift_block = "";
		string response = "";

		for (size_t i = 0; i < buf_vec.size(); i++)
		{
			if (app_id == buf_vec[i])
			{
				num_house = house_vec[i];
				comp_id = comp_vec[i];
				lift_block = lift_vec[i];
				break;
			}
		}

		///___search for mqtt_____/////
		boost::json::array array_tokens = __buf_json_recive.at("request").at("nlu").at("tokens").as_array();
		vector<string> search_mqtt;
		bool flag_mqtt = false;
		for (size_t i = 0; i < array_tokens.size(); i++)
		{
			search_mqtt.push_back(boost::json::serialize(array_tokens[i]));
			if (array_tokens[i].as_string() == "этаж" || array_tokens[i].as_string() == "подъем" || array_tokens[i].as_string() == "спуск" || array_tokens[i].as_string() == "подними" || array_tokens[i].as_string() == "опусти")
			{
				flag_mqtt = true;
			}
		}
		if (flag_mqtt)
		{
			one_table.clear();
			one_table = __db_info.at("House");
			buf_vec.clear(); house_vec.clear(); comp_vec.clear();
			house_vec = __db_info.at("House").at("HouseNum");
			comp_vec = __db_info.at("House").at("ComplexId");
			vector<string> top_floor = __db_info.at("House").at("TopFloor");
			vector<string> bot_floor = __db_info.at("House").at("BottomFloor");
			vector<string> null_floor = __db_info.at("House").at("NullFloor");
			string bufNum = "";
			int floor;
			for (size_t i = 0; i < __key_roots.size(); i++)
			{
				for (size_t j = 0; j < search_mqtt.size(); j++)
				{
					if (search_mqtt[j].find(__key_roots[i]) != search_mqtt[j].npos)
					{
						bufNum += search_mqtt[j];
					}
				}
			}
			int numFloor = __num_roots.at(bufNum);

			for (size_t i = 0; i < house_vec.size(); i++)
			{
				if (house_vec[i] == num_house && comp_vec[i] == comp_id)
				{
					if (top_floor[i] == to_string(numFloor) || bot_floor[i] == to_string(numFloor) || null_floor[i] == to_string(numFloor))
					{
						response = "перемещаю вас на " + bufNum + "этаж";
						break;
					}
				}
			}
			__buf_send = boost::json::serialize(json_formatter::worker::response::marussia_mqtt_message(__name, app_id, __getRespToMS(response), lift_block, numFloor));
		}
		else
		{
			///_____static phrases response static phrase___///

			string command = boost::json::serialize(__buf_json_recive.at("request").at("command"));
			one_table.clear();
			one_table = __db_info.at("StaticPhrases");
			buf_vec.clear(); house_vec.clear(); comp_vec.clear();
			buf_vec = __db_info.at("StaticPhrases").at("KeyWords");
			house_vec = __db_info.at("StaticPhrases").at("HouseNumber");
			comp_vec = __db_info.at("StaticPhrases").at("ComplexId");

			vector<string> resp = __db_info.at("StaticPhrases").at("Response");

			response.clear();

			for (size_t i = 0; i < buf_vec.size(); i++)
			{
				if (command == buf_vec[i])
				{
					if (num_house == house_vec[i] && comp_id == comp_vec[i])
					{
						response = resp[i];
						break;
					}
				}
			}
			if (response != "")
			{
				__buf_send = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, app_id, __getRespToMS(response)));
			}
			else
			{
				response = "Извините, я не знаю такой комманды, попробуйте перефразировать";
				__buf_send = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, app_id, __getRespToMS(response)));
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

