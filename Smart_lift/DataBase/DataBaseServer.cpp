#include "Modules/Libraries/sqlite3.h"
#include "Modules/Libraries/sqlite3ext.h"
#include "DataBaseServer.hpp"
/*
#include "Modules/Client/Client.hpp"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/read.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void fail(beast::error_code ErrCode, char const* what)
{
	cerr << what << ": " << ErrCode.message() << endl;
}


void DataBaseServer::Run()
{
	cout << "Server is working" << endl;
	net::dispatch(socket.get_executor(),boost::bind(&DataBaseServer::DoRead, shared_from_this()));
	if (socket.is_open())
	{
		cout << "open" << endl;
	}
	else
	{
		cout << "error" << endl;
	}
	
}

void DataBaseServer::DoRead()
{
	message.clear();
		if (socket.is_open())
	{
		cout << "open" << endl;
	}
	else
	{
		cout << "error" << endl;
	}
	net::async_read(socket, net::buffer(message), boost::bind(&DataBaseServer::OnRead, this, boost::placeholders::_1, boost::placeholders::_2));
}

void DataBaseServer::OnRead(beast::error_code errorCode, size_t bytesTransferred)
{
	boost::ignore_unused(bytesTransferred);
	if (errorCode == http::error::end_of_stream)
	{
		return DoClose();
	}
	if (errorCode)
	{
		return fail(errorCode, "readServer");
	}

	SendResponse(message);
}

void DataBaseServer::SendResponse(string message)
{
	message = "hui";
	net::async_write(socket, net::buffer(message, message.size()), boost::bind(&DataBaseServer::OnWrite, this, boost::placeholders::_1, boost::placeholders::_2));
}

void DataBaseServer::OnWrite(beast::error_code errorCode, size_t bytesTransferred)
{
	if (errorCode)
	{
		return fail(errorCode, "write");
	}
	DoRead();
}

void DataBaseServer::DoClose()
{
	beast::error_code errorCode;
	socket.shutdown(tcp::socket::shutdown_send, errorCode);
}
void Listener::Run()
{

	__DoAccept();
}
void Listener::__DoAccept()
{
	acceptor.async_accept(net::make_strand(ioc), beast::bind_front_handler(&Listener::__OnAccept, shared_from_this()));
}

void Listener::__OnAccept(beast::error_code errorCode, tcp::socket socket)
{
	if (errorCode)
	{
		fail(errorCode, "accept");
		return;
	}
	else
	{
		make_shared<DataBaseServer>(move(socket), doocRoot)->Run();
	}
	__DoAccept();
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".ACP");

	try
	{

		/*if (argc != 4)
		{
			std::cerr << "Usage: " << argv[0] << " <address> <port> <threads>\n";
			std::cerr << "  For IPv4, try:\n";
			std::cerr << "    receiver 0.0.0.0 80 1\n";
			std::cerr << "  For IPv6, try:\n";
			std::cerr << "    receiver 0::0 80 1\n";
			return EXIT_FAILURE;
		}
		auto const address = net::ip::make_address(argv[1]);
		unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));
		auto const threads = max<int>(1, atoi(argv[3]));

		string add = "0.0.0.0";
		string p = "80";
		const char* addr = "127.0.0.1";
		const char* por = "8001";
		string doocRoot = ".";
		auto const address = net::ip::make_address(add);
		//auto const address = tcp::v4();
		auto const port = static_cast<unsigned short>(atoi("8001"));
		auto const docRoot = make_shared<string>(doocRoot);
		auto const threads = max<int>(1, 2);
		net::io_context ioc{threads};
		make_shared<Listener>(ioc, tcp::endpoint{address, port}, docRoot)->Run();
		cout << 4 << endl;
		vector<thread> v;
		v.reserve(threads - 1);
		for (auto i = threads - 1; i > 0; i--)
		{
			v.emplace_back([&ioc]
				{
					ioc.run();
					cout << "1" << endl;
				}
			);
		}
		string message = "hellow";
		cout << 5 << endl;
		make_shared<Client>(ioc, message)->RunClient();

		cout << 6 << endl;
		ioc.run();
		cout << "7" << endl;
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}*/

int main()
{
	cout << "Hi" << endl;
}