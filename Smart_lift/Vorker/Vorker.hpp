#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
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
#include <map>
#include <fstream>

#include "Modules/ClientMtW/Client.hpp"
#include "../DataBase/Modules/Libraries/sqlite3.h"

#define DB_FILE_WAY "..\\..\\..\\..\\..\\Smart_lift\\DataBaseFile\\SmartLiftBase.db"

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

beast::string_view
mime_type(beast::string_view path);

string PathCat(beast::string_view base, beast::string_view path);
template <class Body, class Allocator>
http::message_generator
HandleReq(
    beast::string_view root,
    http::request<Body, http::basic_fields<Allocator>>&& req);

void fail(beast::error_code ErrCode, char const* what);
string Json(string body);

class HttpSession : public enable_shared_from_this<HttpSession>
{

    beast::tcp_stream stream;
    beast::flat_buffer buffer;
    shared_ptr<string const> root;
    http::request < http::string_body> request;

public:
    HttpSession(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root) : stream(std::move(socket)), root(doc_root)
    {
        cout << 12121 << endl;
    }
    //Log logsWorkerM;
    void Run();
    void DoRead();
    void OnRead(beast::error_code errorCode, size_t bytesTransferred);
    void SendResponse(http::message_generator&& message);
    void OnWrite(bool keep_alive, beast::error_code errorCode, size_t bytesTransferred);
    void DoClose();
    //void setConfig(map<string, string> config);
    //void getConfig();
    //void configData();
    //void setLogs();
    void chooseResponce();
    bool checkConnect();
    void readJson();
    void writeJson();
    static int callback(void* NotUsed, int argc, char** argv, char** azColName);

private:
    //map<string, string> __config;
    boost::property_tree::ptree __root;
    sqlite3* __dB;
    string __query;
    static string __reqRes;
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
    }
    void Run();

private:
    void __DoAccept();
    void __OnAccept(beast::error_code errorCode, tcp::socket socket);
};



