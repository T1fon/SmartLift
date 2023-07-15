/*#include "Client.hpp"

void failClient(beast::error_code ec, char const* what)
{
	cerr << what << ": " << ec.message() << endl;
}

void Client::RunClient()
{
	auto const address = net::ip::make_address(__host);
	auto const port = static_cast<unsigned short>(atoi(__port));
	cout << address << " " << port << endl;
	tcp::endpoint eP(address, port);
	//tcp::resolver::query q(__host, __port);
	__socket.async_connect(eP, boost::bind(&Client::OnConnect, this, boost::placeholders::_1));
	//net::async_connect(__socket, eP, boost::bind(&Client::OnConnect, this, boost::placeholders::_1));

}
void Client::OnConnect(boost::system::error_code ec)
{
	if (ec)
	{
		cout <<ec << endl;
		return failClient(ec, "ClientConnect");
	}
	__socket.async_write_some(net::buffer(__message, __message.size()), boost::bind(&Client::OnWrite, this, boost::placeholders::_1, boost::placeholders::_2));
}

void Client::OnWrite(boost::system::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);
	if (ec)
	{
		return failClient(ec, "write");
	}
	net::async_read(__socket, net::buffer(__message), boost::bind(&Client::onRead, this, boost::placeholders::_1, boost::placeholders::_2));
}
void Client::onRead(boost::system::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);
	if (ec)
	{
		return failClient(ec, "read");
	}
	cout << __message << endl;
	__socket.shutdown(tcp::socket::shutdown_both, ec);
	if (ec && ec != beast::errc::not_connected)
	{
		return failClient(ec, "shutdown");
	}
}*/