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

class Client
{
public:
    Client(net::io_context& ioc, tcp::resolver::iterator EndPointIter, string message);

private:
   net::io_context& __ioc;
    tcp::socket __socket;

    string __message;
    static const size_t __bufLen = 1024;
    string __writeMessage;

    void __onConnect(const boost::system::error_code& ErrorCode);
    void __onReceive(const boost::system::error_code& ErrorCode);
    void __onSend(const boost::system::error_code& ErrorCode);
    void __doClose();
};
