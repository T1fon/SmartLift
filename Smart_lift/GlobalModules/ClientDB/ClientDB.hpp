#pragma once

#include "../JSONFormatter/JSONFormatter.hpp"
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
	
	typedef std::function<void(map<string, map<string, vector<string>>>)> callback_t;

	ClientDB(string ip_dB, string port_dB, string worker_log, string worker_pass, string name_sender, shared_ptr<tcp::socket> socket, callback_t callback)
	{
		__callback_f = callback;
		__end_point = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ip_dB), stoi(port_dB)));
		__socket = socket;
		__buf_recive = new char[BUF_RECIVE_SIZE + 1];
		fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
		__buf_json_recive = {};
		__parser.reset();
		__worker_login = worker_log;
		__worker_password = worker_pass;
		__name_sender = name_sender;
		__flag_conditions = false;

	}
	~ClientDB()
	{
		this->stop();
		delete[] __buf_recive;
	}
	void start()
	{
		__flag_connet = false;
		__flag_disconnect = false;
		__socket->async_connect(*__end_point, boost::bind(&ClientDB::__checkConnect, shared_from_this(), boost::placeholders::_1));
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
	}
	void setQuerys(queue<string> queue_tables, queue<vector<string>> queue_fields, queue<string> queue_conditions )
	{
		__queue_tables = queue_tables;
		__queue_fields = queue_fields;

		if(!queue_conditions.empty())
		{
			__queue_conditions = queue_conditions;
			//cerr << "cond " << __queue_conditions.size() << " tables" << __queue_tables.size() << endl;
			if (__queue_conditions.size() != __queue_tables.size())
			{
				cerr << "ERROR, Not full conditions WHERE( if WHERE is not needed, put " ")";
				this->stop();
			}
			else
			{
				__flag_conditions = true;
			}
		}
	}

	void setCallback(callback_t callback) {
		__callback_f = callback;
	}
	map<string, map<string, vector<string>>> getRespData()
	{
		return __resp_data;
	}

private:
	void __emptyCallback(map<string, map<string, vector<string>>> data)
	{
		cerr << "ClientDb no Data to Worker" << endl;
	}
	void __checkConnect(const boost::system::error_code& error_code)
	{
		if (error_code)
		{
			//cerr << error_code	.what() << endl;
			#ifndef UNIX
				sleep(2);
			#else
				Sleep(2000);
			#endif
			this->stop();
			this->start();
			return;
		}
		if (__flag_connet == false)
		{
			if (__flag_disconnect == true)
			{
				cerr << "no connect" << endl;
				this->stop();
			}
			else
			{
				__buf_queue_string = boost::json::serialize(json_formatter::database::request::connect(__name_sender, __worker_login, __worker_password));
				__socket->async_send(boost::asio::buffer(__buf_queue_string, __buf_queue_string.size()), boost::bind(&ClientDB::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
		}
		else
		{
			__queryGenerator();
		}
	}
	void __queryGenerator()
	{
		if (__flag_disconnect == false)
		{
			if (!__queue_tables.empty())
			{
				__table_name_send = __queue_tables.front();
				__fields_name_send = __queue_fields.front();
				__queue_tables.pop();
				__queue_fields.pop();
				string query = "SELECT ";
				for (size_t i = 0; i < __fields_name_send.size() - 1; i++)
				{
					query += __fields_name_send[i] + ", ";
				}
				query += __fields_name_send[__fields_name_send.size() - 1];
				query += " FROM " + __table_name_send;
				if (__flag_conditions)
				{
					query += " ";
					string conditions = __queue_conditions.front();
					__queue_conditions.pop();
					query += conditions;
					if (__queue_conditions.empty())
					{
						__flag_conditions = false;
					}
				}
				//cerr << query << endl;
				__buf_queue_string = boost::json::serialize(json_formatter::database::request::query(__name_sender, json_formatter::database::QUERY_METHOD::SELECT, __fields_name_send, query));
				__socket->async_send(boost::asio::buffer(__buf_queue_string, __buf_queue_string.size()), boost::bind(&ClientDB::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
			else
			{
				__buf_queue_string = boost::json::serialize(json_formatter::database::request::disconnect(__name_sender));
				__socket->async_send(boost::asio::buffer(__buf_queue_string, __buf_queue_string.size()), boost::bind(&ClientDB::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
		}
		else
		{
			__callback_f(__resp_data);
			this->stop();
		}

	}
	void __sendCommand(const boost::system::error_code& error_code, size_t bytes_send)
	{
		if (error_code)
		{
			//cerr << "sendConnect" << error_code.what() << endl;
			this->stop();
			this->start();
			return;
		}
		static size_t temp_bytes_send = 0;
		/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
			return;
		}*/
		temp_bytes_send += bytes_send;
		if (__buf_queue_string.size() != temp_bytes_send) {
			__socket->async_send(boost::asio::buffer(__buf_queue_string.c_str() + temp_bytes_send, (__buf_queue_string.size() - temp_bytes_send)),
				boost::bind(&ClientDB::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_bytes_send = 0;
		__buf_queue_string.clear();
		__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void __reciveCommand(const boost::system::error_code& error_code, size_t bytes_recive)
	{
		if (error_code)
		{
			cerr << "reciveCommand " << error_code.what() << endl;
			this->stop();
			this->start();
			return;
		}
		/*if (__reciveCheck(bytesRecive, boost::bind(&Client::__reciveCommand, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL)
		{
			return;
		}*/
		try {
			__parser.write(__buf_recive, bytes_recive);
		}
		catch (exception& e) {
			cerr << e.what() << endl;
		}
		fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);

		if (!__parser.done()) {
			cerr << "connectAnalize json not full" << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		try {
			__parser.finish();
			__buf_json_recive = __parser.release();
			__parser.reset();
		}
		catch (exception& e) {
			__parser.reset();
			__buf_json_recive = {};
			cerr << "_reciveCheck " << e.what();
			return;
		}
		__commandAnalize(error_code);
	}
	void __commandAnalize(const boost::system::error_code& error_code)
	{
		boost::json::value target = __buf_json_recive.at("target");
		//cerr << __buf_json_recive << endl;
		if (target == "connect")
		{
			boost::json::value status = __buf_json_recive.at("response").at("status");
			//cout << "status " << status << endl;
			{
				if (status != "success")
				{
					__flag_disconnect = true;
					__checkConnect(error_code);
				}
				else
				{
					__flag_connet = true;
					__flag_disconnect = false;
					__checkConnect(error_code);

				}
			}
		}
		else if (target == "disconnect")
		{
			boost::json::value status = __buf_json_recive.at("response").at("status");
			if (status == "success")
			{
				__flag_disconnect = true;
				__checkConnect(error_code);
			}
			else
			{
				__flag_disconnect = false;
				__checkConnect(error_code);
			}
		}
		else if (target == "db_query")
		{
			map<string, vector<string>> bufRespMap;
			vector<string> valueMap;
			bool whronge = false;
			try
			{
				whronge = false;
				for (size_t i = 0; i < __fields_name_send.size(); i++)
				{
					valueMap.clear();
					boost::json::array valueJson = __buf_json_recive.at("response").at(__fields_name_send[i]).as_array();
					for (size_t j = 0; j < valueJson.size(); j++)
					{
						valueMap.push_back(boost::json::serialize(valueJson[j]));
					}
					bufRespMap[__fields_name_send[i]] = valueMap;
				}
				__resp_data[__table_name_send] = bufRespMap;
			}
			catch (exception& e)
			{
				whronge = true;
				//cout << "No " << __table_name_send << endl;
			}
			__checkConnect(error_code);
		}
	}

	callback_t __callback_f;
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	shared_ptr<tcp::endpoint> __end_point;
	shared_ptr<tcp::socket> __socket;
	static const int BUF_RECIVE_SIZE = 2048;
	queue<string> __queue_tables;
	queue<vector<string>> __queue_fields;
	queue<string> __queue_conditions;
	string __buf_queue_string;
	char* __buf_recive;
	boost::json::value __buf_json_recive;
	boost::json::stream_parser __parser;
	string __resp_error;
	map<string, map<string, vector<string>>> __resp_data;
	string __worker_login;
	string __worker_password;
	string __table_name_send;
	vector<string> __fields_name_send;
	bool __flag_connet;
	bool __flag_disconnect;
	bool __flag_conditions;
	string __name_sender;

	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __reciveCheck(const size_t& count_recive_byte, __handler_t&& handler);
	__CHECK_STATUS __sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);

};
