#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <boost/locale.hpp>
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

namespace https_server {

    namespace json = boost::json;
    namespace beast = boost::beast;         // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace net = boost::asio;            // from <boost/asio.hpp>
    namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
    using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

    //------------------------------------------------------------------------------
    void fail(beast::error_code ec, char const* what);
    //------------------------------------------------------------------------------

    class Session : public std::enable_shared_from_this<Session>
    {
        beast::ssl_stream<beast::tcp_stream> __stream;
        beast::flat_buffer __buffer;
        http::request<http::string_body> __req;
        json::stream_parser __parser;
        json::value __body_request;
        json::object __body_response;
        bool __is_live = false;

        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_mqtt;
        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_marussia;

    public:
        explicit Session(tcp::socket&& socket,
            ssl::context& ssl_ctx,
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

        void __analizeRequest();
        http::message_generator __badRequest(beast::string_view why);
        void __callbackWorkerMarussia(boost::system::error_code error, boost::json::value data);
        void __callbackWorkerMQTT(boost::system::error_code error, boost::json::value data);
        void __constructResponse(boost::json::object response_data);
    };

    //------------------------------------------------------------------------------

    class Listener : public std::enable_shared_from_this<Listener>
    {
        const int __TIME_DEAD_SESSION = 30;
        net::io_context& __io_ctx;
        ssl::context& __ssl_ctx;
        std::shared_ptr<tcp::acceptor> __acceptor;
        std::shared_ptr<std::vector<std::shared_ptr<https_server::Session>>> __sessions;
        std::shared_ptr<boost::asio::deadline_timer> __timer_kill;

        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_mqtt;
        std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> __sessions_marussia;
    public:
        Listener( net::io_context& io_ctx,
                  ssl::context& ssl_ctx,
                    unsigned short port,
                  std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
                  std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia);
        void start();
    
    private:
        void __onAccept(beast::error_code ec, tcp::socket socket);
        void __killSession(beast::error_code ec);
    };
}