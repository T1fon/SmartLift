#include "Client.hpp"

void failClient(beast::error_code ec, char const* what)
{
	cerr << what << ": " << ec.message() << endl;
}

Client::Client(net::io_context& ioc, tcp::resolver::iterator EndPointIter, string message) : __ioc(ioc), __socket(ioc), __writeMessage(message)
{
	tcp::endpoint eP = *EndPointIter;
	__socket.async_connect(eP, boost::bind(&Client::__onConnect, this, net::placeholders::error));
}

void Client::__onConnect(const boost::system::error_code& ErrorCode)
{
	if (ErrorCode)
	{
		return failClient(ErrorCode, "ConnectClient");
	}
	__socket.async_write_some(net::buffer(__writeMessage, __writeMessage.length()), boost::bind(&Client::__onReceive, this, net::placeholders::error));
}

void Client::__onReceive(const boost::system::error_code& ErrorCode)
{
	if (ErrorCode)
	{
		return failClient(ErrorCode, "writeClient");
	}
	__socket.async_read_some(net::buffer(__message, __bufLen), boost::bind(&Client::__onSend, this, net::placeholders::error));
}
void Client::__onSend(const boost::system::error_code& ErrorCode)
{
	if (ErrorCode)
	{
		return failClient(ErrorCode, "readClient");
	}
	__doClose();
}

void Client::__doClose()
{
	__socket.close();
}