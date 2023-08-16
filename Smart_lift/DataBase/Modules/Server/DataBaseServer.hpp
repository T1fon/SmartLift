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

#define DB_WAY "./SmartLiftBase.db"

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;


class DataBase : public enable_shared_from_this<DataBase>
{
private:

	const string __name = "Data_base";
	shared_ptr<tcp::endpoint>__end__point;
	shared_ptr<tcp::socket>__socket;

	static const int BUF_SIZE = 2048;
	string __buf_send;
	char* __buf_recieve;
	boost::json::value __buf_json_recieve;
	boost::json::stream_parser __parser;
	sqlite3* __dB;
	string __query;
	string __error_what;
	shared_ptr<Log> __log;
	map<string, vector<string>> __answer;
	bool __flag_wrong_connect = false;

	void __reqAutentification()
	{
		cerr << "hehehe0" << endl;
		/*if (eC)
{
	cerr << eC.message() << endl;
	Sleep(2000);
	this->start();
	__log->writeLog(3, "DataBase", "Error_connect");
	__log->writeTempLog(3, "DataBase", "Error_connect");
	return;
}		*/
		__buf_send = "";
		__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __resAutentification(const boost::system::error_code& eC, size_t bytes_send)
	{
		cerr << "hehe" << endl;
		if (eC)
		{
			__log->writeLog(2, "DataBase", "Error_failed_to_read_response");
			__log->writeTempLog(2, "DataBase", "Error_failed_to_read_response");
			cerr << eC.message() << endl;
			Sleep(2000);
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__resAutentification, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
			cerr << "nehehe" << endl;
		}
		static size_t temp_send = 0;
		temp_send += bytes_send;
		if (temp_send != __buf_send.size())
		{
			__log->writeTempLog(0, "DataBase", "Not_full_json");
			__socket->async_send(net::buffer(__buf_send.c_str() + temp_send, __buf_send.size() - temp_send), boost::bind(&DataBase::__resAutentification, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_send = 0;
		__buf_send.clear();
		__log->writeTempLog(2, "DataBase", "Receiving_a_request");
		__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __connectAnalize(const boost::system::error_code& eC, size_t bytes_recieve)
	{
		cout << "analize" << endl;
		if (eC)
		{
			cerr << eC.message() << endl;
			Sleep(1000);
			__log->writeLog(2, "DataBase", "Error_failed_to_read_request");
			__log->writeTempLog(2, "DataBase", "Error_failed_to_read_request");
			__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try
		{
			__parser.write(__buf_recieve, bytes_recieve);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
		fill_n(__buf_recieve, BUF_SIZE, 0);
		if (!__parser.done())
		{
			cerr << "Connect analize json not full" << endl;
			__log->writeTempLog(2, "DataBase", "Not_full_json");
			__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try
		{
			__parser.finish();
			__buf_json_recieve = __parser.release();
			__parser.reset();

			string login = boost::json::serialize(__buf_json_recieve.at("request").at("login"));
			cout << "login" << login << endl;
			string password = boost::json::serialize(__buf_json_recieve.at("request").at("password"));
			cout << "password" << password << endl;
			__checkConnect(login, password);
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__resAutentification, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			if (__flag_wrong_connect)
			{
				this->stop();
				this->start();
				__flag_wrong_connect = false;
			}
			__log->writeTempLog(0, "DataBase", "send_answer_to_connect");

		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
	}
	void __waitCommand(const boost::system::error_code& eC, size_t bytes_recieve)
	{
		cout << "wait" << endl;
		if (eC) {
			__log->writeLog(2, "DataBase", "Error_failed_to_read_request");
			__log->writeTempLog(2, "DataBase", "Error_failed_to_read_request");
			cerr << eC.message() << endl;
			Sleep(1000);
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		try
		{
			__parser.write(__buf_recieve, bytes_recieve);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
		fill_n(__buf_recieve, BUF_SIZE, 0);
		if (!__parser.done())
		{
			cerr << "waitCommand json is not full" << endl;
			__log->writeTempLog(0, "DataBase", "Not_full_json");
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return; //not full Json
		}
		try
		{
			__parser.finish();
			__buf_json_recieve = __parser.release();
			__parser.reset();
		}
		catch (exception& e)
		{
			__log->writeLog(2, "DataBase", "Error_failed_to_read_request");
			__log->writeTempLog(2, "DataBase", "Error_failed_to_read_request");
			cerr << e.what() << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			__error_what.clear();
			__error_what = e.what();
			return; //exception
		}

		string command = __checkCommand(__buf_recieve, bytes_recieve);
		if (command == "ping")
		{
			__log->writeTempLog(0, "DataBase", "Make_ping_response");
			__makePing();
		}
		else if (command == "disconnect")
		{
			__log->writeTempLog(0, "DataBase", "Make_disconnect_response");
			__makeDisconnect();
		}
		else if (command == "db_query")
		{
			__log->writeTempLog(0, "DataBase", "Make_query_response");
			__makeQuery();
		}
		else if (command == "error")
		{
			__log->writeTempLog(0, "DataBase", "Make_error_response");
			__makeError();
		}
		else if (command == "connect")
		{
			cout << "connect" << endl;
			__log->writeTempLog(0, "DataBase", "Start_connect");
			__connectAnalize(eC, bytes_recieve);
		}
		__log->writeLog(0, "DataBase", "Send_response_command");
		__log->writeTempLog(0, "DataBase", "Send_response_command");
		__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __sendCommand(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			__log->writeLog(2, "DataBase", "Error_send_response_command");
			__log->writeTempLog(2, "DataBase", "Error_send_response_command");
			cerr << eC.message() << endl;
			Sleep(2000);
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		static size_t temp_send = 0;
		temp_send += bytes_send;
		if (temp_send != __buf_send.size())
		{
			__log->writeTempLog(0, "DataBase", "Not_full_json");
			__socket->async_send(net::buffer(__buf_send.c_str() + temp_send, __buf_send.size() - temp_send), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_send = 0;
		__buf_send.clear();
		if (__flag_wrong_connect)
		{
			__flag_wrong_connect = false;
			stop();
		}
		else

		{
			__log->writeTempLog(0, "DataBase", "Recieve_request");
			__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
	}

	string __checkCommand(char* __bufRecieve, size_t bytes_recieve)
	{
		cerr << __buf_json_recieve.at("target") << endl;
		if (__buf_json_recieve.at("target") == "ping")
		{
			return "ping";
		}
		else if (__buf_json_recieve.at("target") == "disconnect")
		{
			return "disconnect";
		}
		else if (__buf_json_recieve.at("target") == "db_query")
		{
			return "db_query";
		}
		else if (__buf_json_recieve.at("target") == "connect")
		{
			return "connect";
		}
		else
		{
			__log->writeTempLog(2, "DataBase", "No_command");
			__error_what.clear();
			__error_what = "non-existent command";
			return "error";//no command
		}
	}
	void __makePing()
	{
		__buf_send.clear();
		__buf_send = boost::json::serialize(json_formatter::database::response::ping(__name));
	}
	void __makeDisconnect()
	{
		__buf_send.clear();
		__buf_send = boost::json::serialize(json_formatter::database::response::disconnect(__name));
		__flag_wrong_connect = true;
	}
	void __makeQuery()
	{
		boost::json::array jsonFields = __buf_json_recieve.at("request").at("fields").as_array();
		vector<string> fields;
		for (size_t i = 0; i < jsonFields.size(); i++)
		{
			fields.emplace_back(jsonFields[i].as_string());
		}
		__query.clear();

		__answer.clear();
		__query = __buf_json_recieve.at("request").at("query").as_string();
		cout << "query " << __query << endl;

		int exit = sqlite3_open(DB_WAY, &__dB);
		int flag = sqlite3_exec(__dB, __query.c_str(), __connection, (void*)&__answer, NULL);
		sqlite3_close(__dB);

		map<string, vector<string>> response;
		map<string, vector<string>>::iterator it = __answer.begin();

		for (size_t i = 0; i < fields.size(); i++)
		{
			vector<string> buf = __answer.at(fields[i]);
			response[fields[i]] = buf;
		}

		__buf_send.clear();
		__buf_send = boost::json::serialize(json_formatter::database::response::query(__name, json_formatter::database::QUERY_METHOD::SELECT, response));
		cout << __buf_send << endl;
	}
	void __makeError()
	{
		__buf_send.clear();
		//__bufSend = boost::json::serialize(json_formatter::database::) //íåò êîìàíäû
	}
	void __checkConnect(string login, string password)
	{
		int exit = sqlite3_open(DB_WAY, &__dB);
		__query.clear();
		__query = "SELECT * FROM Accounts";
		__buf_send.clear();
		__answer.clear();
		char* error_msg = 0;
		cout << "query " << __query << endl;
		int flag = sqlite3_exec(__dB, __query.c_str(), __connection, (void*)&__answer, &error_msg);
		if (flag != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", error_msg);
			sqlite3_free(error_msg);
			cout << "not ok exec" << endl;
			__buf_send = boost::json::serialize(json_formatter::database::response::connect(__name, json_formatter::ERROR_CODE::CONNECT, "User not found"));
			__flag_wrong_connect = true;
		}
		else
		{
			try
			{
				vector<string> __login = __answer.at("Login");
				vector<string> __password = __answer.at("Password");
				int num_pus = -1;
				for (size_t i = 0; i < __login.size(); i++)
				{
					string buf_log = "\"" + __login[i] + "\"";
					
					if (buf_log == (string)login)
					{
						num_pus = i;
						break;
					}
				}
				string buf_pas = "\"" + __password[num_pus] + "\"";
				if (buf_pas == password)
				{
					__buf_send = boost::json::serialize(json_formatter::database::response::connect(__name));
				}
				else
				{
					cout << "flag" << endl;
					__buf_send = boost::json::serialize(json_formatter::database::response::connect(__name, json_formatter::ERROR_CODE::CONNECT, "Password mismatch"));
					__flag_wrong_connect = true;
				}
			}
			catch (exception& e)
			{
				cerr << e.what() << endl;
			}
		}
		sqlite3_close(__dB);
	}
	static int __connection(void* answer, int argc, char** argv, char** az_col_name)
	{
		try
		{
			map<string, vector<string>>* buf_map = (map<string, vector<string>>*)answer;
			for (size_t i = 0; i < argc; i++)
			{
				string buf_answer = argv[i];
				string col_name = az_col_name[i];
				vector<string> buf_vec;
				try
				{
					buf_vec = buf_map->at(col_name);
					buf_vec.push_back(buf_answer);
				}
				catch (exception e)
				{
					buf_vec.push_back(buf_answer);
				}
				//buf_vec.push_back(buf_answer);
				(*buf_map)[col_name] = buf_vec;
			}
			return 0;
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
	}


public:
	DataBase(shared_ptr<Log> lg, tcp::socket sock)
	{
		cerr << "11" << endl;
		__socket = make_shared<tcp::socket>(move(sock));
		__log = lg;
		__buf_recieve = new char[BUF_SIZE + 1];

		__buf_send = "";
		__buf_json_recieve = {};
		__parser.reset();

		int checkDb = sqlite3_open(DB_WAY, &__dB);
		if (checkDb)
		{
			cerr << "can`t open dataBase" << endl;
			sqlite3_close(__dB);
			return;
		}
		else
		{
			cerr << "Db open" << endl;
			sqlite3_close(__dB);
		}
	}
	void start()
	{
		cerr << 12 << endl;
		__log->writeLog(0, "DataBase", "Connect_with_DataBase");
		__log->writeTempLog(0, "DataBase", "Connect_with_DataBase");
		//__socket->async_connect(*__endPoint, boost::bind(&DataBase::__reqAutentification, shared_from_this(), boost::placeholders::_1));
		__reqAutentification();
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
		__log->writeLog(0, "DataBase", "End_Connect");
		__log->writeTempLog(0, "DataBase", "End_Connect");
	}
	~DataBase()
	{
		this->stop();
		delete[] __buf_recieve;
	}
};

class Server : public enable_shared_from_this<Server>
{
public:
	Server(shared_ptr<net::io_context> io_context, string name_config_file)
	{
		__ioc = io_context;
		__sessions = make_shared<std::vector<std::shared_ptr<DataBase>>>();
		__log_server = make_shared<Log>("Log/", "./", "DataBase");
		__config = make_shared<Config>(__log_server, "./", "", name_config_file);
		__config->readConfig();
		__config_info = __config->getConfigInfo();
		if (__config_info.size() == 0)
		{
			__log_server->writeLog(1, "DataBase", "Config_File_not_open");
			return;
		}
		else if (__config_info.size() < CONFIG_NUM_FIELDS)
		{
			__log_server->writeLog(1, "DataBase", "Config_File_not_full");
		}
		try
		{
			for (size_t i = 0, length = __config_info.size(); i < length; i++)
			{
				__config_info.at(CONFIG_FIELDS.at(i));
			}
		}
		catch (exception& e)
		{
			__log_server->writeLog(1, "DataBase", e.what());
			return;
		}
		string port = __config_info.at("port");
		cerr << port << endl;
		__acceptor = make_shared<tcp::acceptor>(*__ioc, tcp::endpoint(tcp::v4(), stoi(port)));

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
	shared_ptr<Log> __log_server;
	shared_ptr<Config> __config;
	map<string, string> __config_info;
	static const int CONFIG_NUM_FIELDS = 1;
	vector<string> CONFIG_FIELDS = { "port" };
	shared_ptr<tcp::acceptor> __acceptor;
	shared_ptr<net::io_context> __ioc;
	std::shared_ptr<std::vector<std::shared_ptr<DataBase>>> __sessions;

	void __doAccept()
	{
		cerr << "01" << endl;
		cerr << "hi" << endl;
		__acceptor->async_accept([this](boost::system::error_code error, tcp::socket socket)
			{
				if (error) {
					cerr << error.message() << endl;
					__doAccept();
				}

				__sessions->push_back(std::make_shared<DataBase>(__log_server, move(socket)));

				__sessions->back()->start();
				__doAccept();
			}
		);
	}
};