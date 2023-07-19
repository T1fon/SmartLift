#include "DataBaseServer.hpp"
#include "Modules/Client/Client.hpp"

void fail(beast::error_code ErrCode, char const* what)
{
	cerr << what << ": " << ErrCode.message() << "  " << endl;
}

DataBaseSession::DataBaseSession(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root): __socket(std::move(socket)), __root(doc_root)
{
	cout << 1 << endl;

}

tcp::socket& DataBaseSession::socket()
{
	cout << 2 << endl;
	return __socket;
}

void DataBaseSession::start()
{
	cout << 3 << endl;
	cout << "server Working" << endl;
	//doRead();
	net::dispatch(__socket.get_executor(), boost::bind(&DataBaseSession::doRead, shared_from_this()));
}

void DataBaseSession::doRead()
{
	cout << 4 << endl;
	__readBoof.clear();
	net::async_read(__socket, net::buffer(__readBoof, __boofLength), boost::bind(&DataBaseSession::onRead, shared_from_this(), net::placeholders::error));
}

void DataBaseSession::onRead(const boost::system::error_code& error)
{
	cout << 5 << endl;
	if (error)
	{
		//close();
		return fail(error, "ReadServer");
	}
	__writeBoof.clear();
	__writeBoof = "hui";
	net::async_write(__socket, net::buffer(__writeBoof, __writeBoof.length()), boost::bind(&DataBaseSession::onWrite, shared_from_this(), net::placeholders::error));
}

void DataBaseSession::onWrite(const::boost::system::error_code& error)
{
	cout << 6 << endl;
	if (error)
	{
		//close();
		return fail(error, "write");
	}
	doRead();
	//close();
}

void DataBaseSession::close()
{
	cout << 7 << endl;
	boost::system::error_code error;
	__socket.shutdown(tcp::socket::shutdown_send, error);
}

Listener::Listener(net::io_context& Ioc, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root)
	: ioc(Ioc)
	, acceptor(net::make_strand(Ioc))
	, doocRoot(doc_root)
{
	cout << 01 << endl;
	boost::system::error_code errorCode;
	acceptor.open(endpoint.protocol(), errorCode);
	if (errorCode)
	{
		fail(errorCode, "open");
		return;
	}

	acceptor.set_option(net::socket_base::reuse_address(true), errorCode);
	if (errorCode)
	{
		fail(errorCode, "set_option");
		return;
	}

	acceptor.bind(endpoint, errorCode);
	if (errorCode)
	{
		fail(errorCode, "bind");
		return;
	}

	acceptor.listen(net::socket_base::max_listen_connections, errorCode);

	if (errorCode)
	{
		fail(errorCode, "listen");
		return;
	}
}

void Listener::run()
{
	cout << 02 << endl;
	__doAccept();
}

void Listener::__doAccept()
{
	cout << 03 << endl;
	acceptor.async_accept(net::make_strand(ioc), beast::bind_front_handler(&Listener::__onAccept, shared_from_this()));
}

void Listener::__onAccept(const boost::system::error_code& error, tcp::socket socket)
{
	cout << 04 << endl;
	if (error)
	{
		fail(error, "accept");
		return;
	}
	else
	{
		make_shared<DataBaseSession>(move(socket), doocRoot)->start();
	}
	cout << 01010 << endl;
	__doAccept();
}


int main()
{
	setlocale(LC_ALL, ".ACP");
	try
	{
		string add = "0.0.0.0";
		string p = "80";
		const char* addr = "127.0.0.1";
		const char* por = "80";
		string doocRoot = ".";
		auto const address = net::ip::make_address(add);
		auto const port = static_cast<unsigned short>(atoi("1234"));
		auto const docRoot = make_shared<string>(doocRoot);
		auto const threads = max<int>(1, 2);
		net::io_context ioc{1};
		string CM = "asasa";

		make_shared<Listener>(ioc, tcp::endpoint{address, port}, docRoot)->run();
		vector<thread> v;
		v.reserve(threads - 1);
		for (auto i = threads - 1; i > 0; i--)
		{
			v.emplace_back([&ioc]
				{
					ioc.run();
				}
			);
		}
		make_shared<Client>(ioc, CM)->RunClient();
		ioc.run();
	}
	catch(exception& e)
	{
		cerr << "Exception " << e.what() << endl;
	}
}