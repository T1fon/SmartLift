#include "Modules/Libraries/sqlite3.h"
#include "Modules/Libraries/sqlite3ext.h"

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
#include <locale.h>
#include <ctime>
#include <map>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

/*
void fail(beast::error_code ErrCode, char const* what);

class DataBaseServer : public enable_shared_from_this<DataBaseServer>
{
    
    string message;
    string query;
    bool keepAlive;
    tcp::socket socket;
    shared_ptr<string const> root;
    //http::request < http::string_body> request;

public:
    DataBaseServer(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root) : socket(std::move(socket)), root(doc_root)
    {
    }
    void Run();
    void DoRead();
    void OnRead(beast::error_code errorCode, size_t bytesTransferred);
    void SendResponse(string message);
    void OnWrite(beast::error_code errorCode, size_t bytesTransferred);
    void DoClose();
};

class Listener : public enable_shared_from_this<Listener>
{
    net::io_context& ioc;
    tcp::acceptor acceptor;
    shared_ptr<string const> doocRoot;

public:
    Listener(net::io_context& Ioc, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root)
        : ioc(Ioc)
        , acceptor(net::make_strand(Ioc))
        , doocRoot(doc_root)
    {
        beast::error_code errorCode;
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
        cout << endpoint.address() << endpoint.port() << endl;
    }
    void Run();

private:
    void __DoAccept();
    void __OnAccept(beast::error_code errorCode, tcp::socket socket);
};*/
