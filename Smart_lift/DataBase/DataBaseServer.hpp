#include "Modules/Libraries/sqlite3.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/placeholders.hpp>
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

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void fail(beast::error_code ErrCode, char const* what);


class DataBaseSession : public boost::enable_shared_from_this<DataBaseSession>
{
public:
	DataBaseSession(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root);
	tcp::socket& socket();
	void start();
	//void doRead(const boost::system::error_code& error);
	void doRead();
	void onRead(const boost::system::error_code& error);
	void onWrite(const::boost::system::error_code& error);
	void close();
private:
	tcp::socket __socket;
	string __readBoof;
	string __writeBoof;
	int __boofLength = 2048;
	shared_ptr<string const> __root;
};

class Listener : public enable_shared_from_this<Listener>
{
public:
	Listener(net::io_context& Ioc, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root);
	net::io_context& ioc;
	tcp::acceptor acceptor;
	shared_ptr<string const> doocRoot;
	void run();
private:
	void __doAccept();
	void __onAccept(const boost::system::error_code& error, tcp::socket socket);
};

