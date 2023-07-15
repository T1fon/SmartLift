#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
/*
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <boost/json/object.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
*/
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         
namespace http = beast::http;          
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;
using namespace std;

void failClient(beast::error_code ec, char const* what);

class Client : public enable_shared_from_this<Client>
{
    tcp::resolver resolver;
    beast::tcp_stream stream;
    beast::flat_buffer buffer; 
    http::request<http::string_body> req;
    http::response<http::string_body> res;

public:
    explicit Client(net::io_context& ioc) : resolver(net::make_strand(ioc)), stream(net::make_strand(ioc))
    {

    }

    void RunClient(char const* host,char const* port,char const* target,int version, string body);
    void OnResolve(beast::error_code ec, tcp::resolver::results_type results);
    void OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
    void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
    void OnRead(beast::error_code ec, std::size_t bytes_transferred);

};