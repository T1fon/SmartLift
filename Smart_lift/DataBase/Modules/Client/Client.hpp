#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/asio/placeholders.hpp>
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

void failClient(beast::error_code ec, char const* what);

class Client : public boost::enable_shared_from_this<Client>, boost::noncopyable
{
	string __message;
	tcp::socket __socket;
	bool __started;
	string __host = "127.0.0.1";
	const char* __port = "1234";

public:
	explicit Client(net::io_context& ioc, string message) :__socket(ioc), __message(message), __started(true)
	{

	}
	void RunClient();
	void OnConnect(boost::system::error_code ec);
	void OnWrite(boost::system::error_code ec, std::size_t bytes_transferred);
	void onRead(boost::system::error_code ec, std::size_t bytes_transferred);


	
};
