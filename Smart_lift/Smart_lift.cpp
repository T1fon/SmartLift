#pragma once

#include "GlobalModules/JSONFormatter/JSONFormatter.hpp"
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


class DataBase : public enable_shared_from_this<DataBase>
{
private:

	shared_ptr<tcp::endpoint>__end_point;
	shared_ptr<tcp::socket>__socket;

	static const int BUF_SIZE = 2048;
	string __buf_send;
	char* __buf_recieve;
	boost::json::value __buf_json_recieve;
	boost::json::stream_parser __parser;
	string __error_what;
	bool __flag_connect;

	void __startWaitCommand()
	{
		//__buf_send = boost::json::serialize(json_formatter::worker::response::connect("Server"));
		//__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__resResp, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		__socket->async_receive(net::buffer(__buf_recieve, BUF_SIZE), boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void __checkConnect(const boost::system::error_code& eC, size_t bytes_recive)
	{
		if (eC) {
			cerr << eC.message() << endl;
			Sleep(1000);
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		try
		{
			__parser.write(__buf_recieve, bytes_recive);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
		fill_n(__buf_recieve, BUF_SIZE, 0);
		if (!__parser.done())
		{
			cerr << "waitCommand json is not full" << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return; //not full Json
		}
		try
		{
			__parser.finish();
			__buf_json_recieve = __parser.release();
			__parser.reset();
			cerr << __buf_json_recieve << endl;
			__buf_send = boost::json::serialize(json_formatter::worker::response::connect("GS"));
			cerr << __buf_send << endl;
			__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
				boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return; //exception
		}
	}

	void __sendCommand(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			cerr << eC.message() << endl;
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
			cerr << "nehehe" << endl;
		}
		static size_t temp_send = 0;
		temp_send += bytes_send;
		if (temp_send != __buf_send.size())
		{
			__socket->async_send(net::buffer(__buf_send.c_str() + temp_send, __buf_send.size() - temp_send), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_send = 0;
		__buf_send.clear();
		__buf_send = boost::json::serialize(json_formatter::worker::request::ping("GS"));
		cerr << __buf_send << endl;
		__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__recResponse, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void __recResponse(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			cerr << eC.message() << endl;
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
			cerr << "nehehe" << endl;
		}
		static size_t temp_send = 0;
		temp_send += bytes_send;
		if (temp_send != __buf_send.size())
		{
			__socket->async_send(net::buffer(__buf_send.c_str() + temp_send, __buf_send.size() - temp_send), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_send = 0;
		__buf_send.clear();
		__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),boost::bind(&DataBase::recive, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	void recive(const boost::system::error_code& eC, size_t bytes_recive)
	{
		if (eC) {
			cerr << eC.message() << endl;
			Sleep(1000);
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		try
		{
			__parser.write(__buf_recieve, bytes_recive);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
		fill_n(__buf_recieve, BUF_SIZE, 0);
		if (!__parser.done())
		{
			cerr << "waitCommand json is not full" << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return; //not full Json
		}
		try
		{
			__parser.finish();
			__buf_json_recieve = __parser.release();
			__parser.reset();
			/*boost::json::value body = {
									{"request",{
												{"command", "подними на пятый этаж"},
												{"original_utterance", "подними на пятый этаж"},
												{"type", "SimpleUtterance"},
												{"payload",{}},
												{"nlu",{
														"tokens",{
														"какая",
														"очередь",
														"в",
														"столовой"
														}
											
												}}
												}
									},
									{"session",{
												{"session_id", "01bfd28fe3a326-c-2-fea35db06d4-a8930"},
												{"user_id", "f63bc4d9e9c89abe10fbe874b5400b67c0df41f86143ec22629b00be606a1dac"},
												{"skill_id", "5b23aa28b9cbd41ad25-21-2-60c7-121d4b"},
												{"new",false},
												{"message_id",1},
												{"user",{
														"user_id", "c825511e862f23f3728a58cd3b15896cd243c7460237c651944b7499c7c9a425"
														},
												{"application",{
																{"application_id", "f63bc4d9e9c89abe10fbe874b5400b67c0df41f86143ec22629b00be606a1dac"},
																{"application_type", "mobile"}
														}

												}}
												}
									}
									

			};*/
			boost::json::object body = { { "sender", "GS"},
				{ "target", "marussia_station_request" },
				{ "request", {
						{"station_id", 1},
						{"body", {
								{"command", "что есть рядом"}
									}
						}
							}
				}
			};
			__buf_send = boost::json::serialize(body);
			cerr << __buf_send << endl;
			__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
				boost::bind(&DataBase::__recResponse, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recieve, BUF_SIZE),
				boost::bind(&DataBase::__checkConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return; //exception
		}
	}

public:
	DataBase(tcp::socket sock)
	{
		cerr << "11" << endl;
		__socket = make_shared<tcp::socket>(move(sock));
		__buf_recieve = new char[BUF_SIZE + 1];
		__flag_connect = false;
		__buf_send = "";
		__buf_json_recieve = {};
		__parser.reset();

	}
	void start()
	{
		__startWaitCommand();
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
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

		string port = "1443";
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

				__sessions->push_back(std::make_shared<DataBase>(move(socket)));

				__sessions->back()->start();
				__doAccept();
			}
		);
	}
};

class ServerDataBase : public enable_shared_from_this<ServerDataBase>
{
private:
	shared_ptr<net::io_context> __ioc;
	short __countThreads = 2;
	shared_ptr<Server> __server;
public:
	ServerDataBase()
	{
		__ioc = make_shared<boost::asio::io_context>(__countThreads);
		__server = make_shared<Server>(__ioc, "");
	}
	~ServerDataBase()
	{
		stop();
	}
	void start()
	{
		__server->run();

		std::vector<std::thread> v;
		v.reserve(__countThreads - 1);
		for (auto i = __countThreads - 1; i > 0; --i)
			v.emplace_back(
				[this]
				{
					__ioc->run();
				});
		__ioc->run();
	}
	void stop()
	{
		__server->stop();
	}

};



int main()
{
	setlocale(LC_ALL, "UTF-8");
	ServerDataBase sDB;
	sDB.start();
}