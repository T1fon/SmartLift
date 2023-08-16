/*

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

	shared_ptr<tcp::socket> socket = make_shared<tcp::socket>(ioc);
	shared_ptr<ClientDB> Client = make_shared<ClientDB>(ip, port, __bDLog, __bDPas, "Worker_Test", socket);
	Client->setQuerys(tables, fields);
	Client->start();
	ioc.run();
}*/
/*int main(int argc, char** argv)
{
	setlocale(LC_ALL, ".ACP");
	try
	{
		auto const threads = max<int>(1, 2);

		net::io_context ioc{threads};
		make_shared<Server>(ioc, "")->Run();
		vector<thread> v;
		v.reserve(threads - 1);
		string ip = "127.0.0.1";
		string port = "80";
		queue<string> que;
		string request = boost::json::serialize(json_formatter::database::request::connect("Worker", "1", "1"));
		que.push(request);
		make_shared<Server>(ioc, "")->Run();
		for (auto i = threads - 1; i > 0; i--)
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