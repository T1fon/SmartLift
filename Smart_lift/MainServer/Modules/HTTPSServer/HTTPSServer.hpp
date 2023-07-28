#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <string>
#include "../WorkerServer/WorkerServer.hpp"

namespace HTTPS_Server {

    namespace json = boost::json;
    namespace beast = boost::beast;         // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace net = boost::asio;            // from <boost/asio.hpp>
    namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
    using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

    //------------------------------------------------------------------------------

    template <class Body, class Allocator>
    http::message_generator handleRequest( beast::string_view doc_root,
                                           http::request<Body, http::basic_fields<Allocator>>&& req);

    void fail(beast::error_code ec, char const* what);

    //------------------------------------------------------------------------------

    class Session : public std::enable_shared_from_this<Session>
    {
        beast::ssl_stream<beast::tcp_stream> __stream;
        beast::flat_buffer __buffer;
        std::shared_ptr<std::string const> __doc_root;
        http::request<http::string_body> __req;
        json::stream_parser __parser;
        bool __is_live = false;

        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_mqtt;
        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_marussia;

    public:
        explicit Session(tcp::socket&& socket,
            ssl::context& ctx,
            std::shared_ptr<std::string const> const& doc_root,
            std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
            std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia);
        void run();
        bool isLive();
    private:
        void __onRun();
        void __onHandshake(beast::error_code ec);
        void __doRead();
        void __onRead(beast::error_code ec,
                      std::size_t bytes_transferred);
        void __sendResponse(http::message_generator&& msg);
        void __onWrite(bool keep_alive,
                       beast::error_code ec,
                       std::size_t bytes_transferred);
        void __doClose();
        void __onShutdown(beast::error_code ec);

        http::message_generator __handleRequest();
        http::message_generator __badRequest(beast::string_view why);
    };

    //------------------------------------------------------------------------------

    class Listener : public std::enable_shared_from_this<Listener>
    {
        const int __TIME_DEAD_SESSION = 30;
        net::io_context& __ioc;
        ssl::context& __ctx;
        tcp::acceptor __acceptor;
        std::shared_ptr<std::string const> __doc_root;
        std::shared_ptr<std::vector<std::shared_ptr<HTTPS_Server::Session>>> __sessions;
        std::shared_ptr<boost::asio::deadline_timer> __timer_kill;

        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_mqtt;
        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_marussia;
    public:
        Listener( net::io_context& ioc,
                  ssl::context& ctx,
                  tcp::endpoint endpoint,
                  std::shared_ptr<std::string const> const& doc_root,
                  std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
                  std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia);
        void run();
    
    private:
        void __onAccept(beast::error_code ec, tcp::socket socket);
        void __killSession(beast::error_code ec);
    };
}