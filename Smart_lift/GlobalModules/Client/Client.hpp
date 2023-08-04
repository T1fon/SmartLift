#pragma once

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
	/*void __ñallback(map<string, map<string, vector<string>>> data)
	{
		cerr << "ClientDb no Data to Worker" << endl;
	}
	typedef std::function<void(map<string, map<string, vector<string>>>)> _callback_t;
	_callback_t _callback;*/
	ClientDB(string ipBD, string portBD, shared_ptr<tcp::socket> socket)
		//_callback(boost::bind(&ClientDB::__ñallback, this, boost::placeholders::_1))
	{
		//_callback = callback;
		__end_point = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ipBD), stoi(portBD)));
		__socket = socket;
		__buf_recive = new char[BUF_RECIVE_SIZE + 1];
		fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
		__buf_json_recive = {};
		__parser.reset();
	}
	~ClientDB()
	{
		this->stop();
		delete[] __buf_recive;
	}
	void start()
	{
		__socket->async_connect(*__end_point, boost::bind(&ClientDB::__checkConnect, shared_from_this(), boost::placeholders::_1));
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
		if (!__queue_to_send.empty())
		{
			__buf_queue_string = __queue_to_send.front();
			__queue_to_send.pop();
			__socket->async_send(boost::asio::buffer(__buf_queue_string, __buf_queue_string.size()), boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		else
		{
			//_callback(__respData);
			this->stop();

		}
	}
	void __sendConnect(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			cerr << "sendConnect" << eC.what() << endl;
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
				boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_bytes_send = 0;
		__buf_queue_string.clear();
		__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void __reciveCommand(const boost::system::error_code& eC, size_t bytes_recive)
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
		__commandAnalize(eC);
	}
	void __commandAnalize(const boost::system::error_code& eC)
	{
		boost::json::value target = __buf_json_recive.at("target");
		cerr << __buf_json_recive << endl;
		if (target == "connect")
		{
			boost::json::value status = __buf_json_recive.at("response").at("status");
			cout << "status " << status << endl;
			{
				if (status != "success")
				{
					__resp_error.clear();
					__resp_error = __buf_json_recive.as_string();
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
			boost::json::value status = __buf_json_recive.at("response").at("status");
			if (status == "success")
			{
				this->stop();
				return;
			}
			else
			{
				__resp_error.clear();
				__resp_error = __buf_json_recive.as_string();
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
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
				if (__buf_json_recive.at("response").at(__marussia_station_fields[0]) != "")
				{
					for (size_t i = 0; i < __marussia_station_fields.size(); i++)
					{
						valueMap.clear();
						boost::json::array valueJson = __buf_json_recive.at("response").at(__marussia_station_fields[i]).as_array();
						for (size_t j = 0; j < valueJson.size(); j++)
						{
							valueMap.push_back(boost::json::serialize(valueJson[j]));
						}
						bufRespMap[__marussia_station_fields[i]] = valueMap;
					}
					__resp_data["marussiaStation"] = bufRespMap;
				}
			}
			catch (exception& e)
			{
				whronge = true;
				cout << "No station" << endl;
			}
			try
			{
				whronge = false;
				if (__buf_json_recive.at("response").at(__house_fields[0]) != "")
				{
					for (size_t i = 0; i < __house_fields.size(); i++)
					{
						valueMap.clear();
						boost::json::array valueJson = __buf_json_recive.at("response").at(__house_fields[i]).as_array();
						for (size_t j = 0; j < valueJson.size(); j++)
						{
							valueMap.push_back(boost::json::serialize(valueJson[j]));
						}
						bufRespMap[__house_fields[i]] = valueMap;
					}
					__resp_data["houseFields"] = bufRespMap;
				}
			}
			catch (exception& e)
			{
				whronge = true;
				cout << "no house" << endl;
			}
			try
			{
				whronge = false;
				if (__buf_json_recive.at("response").at(__static_phrases_fields[0]) != "")
				{
					for (size_t i = 0; i < __static_phrases_fields.size(); i++)
					{
						valueMap.clear();
						boost::json::array valueJson = __buf_json_recive.at("response").at(__static_phrases_fields[i]).as_array();
						for (size_t j = 0; j < valueJson.size(); j++)
						{
							valueMap.push_back(boost::json::serialize(valueJson[j]));
						}
						bufRespMap[__static_phrases_fields[i]] = valueMap;
					}
					__resp_data["staticPhrases"] = bufRespMap;
				}
			}
			catch (exception& e)
			{
				whronge = true;
				cout << "no static" << endl;
			}
			__checkConnect(eC);
		}
	}
	void setQuery(queue<string> queue)
	{
		__queue_to_send = queue;
	}
	map<string, map<string, vector<string>>> getRespData()
	{
		return __resp_data;
	}

private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	shared_ptr<tcp::endpoint> __end_point;
	shared_ptr<tcp::socket> __socket;
	static const int BUF_RECIVE_SIZE = 2048;
	queue<string> __queue_to_send;
	string __buf_queue_string;
	char* __buf_recive;
	boost::json::value __buf_json_recive;
	boost::json::stream_parser __parser;
	string __resp_error;
	map<string, map<string, vector<string>>> __resp_data;
	vector<string> __marussia_station_fields = { "ApplicationId", "ComplexId", "HouseNum" };
	vector<string> __house_fields = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> __static_phrases_fields = { "ComplexId", "HouseNumber", "KeyWords", "Response" };

	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __reciveCheck(const size_t& count_recive_byte, __handler_t&& handler);
	__CHECK_STATUS __sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);

};
