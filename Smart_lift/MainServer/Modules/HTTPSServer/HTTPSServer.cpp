#include "HTTPSServer.hpp"

using namespace HTTPS_Server;

//------------------------------------------------------------------------------
void HTTPS_Server::fail(beast::error_code ec, char const* what) {
    if (ec == net::ssl::error::stream_truncated) { return; }

    std::cerr << what << ": " << ec.message() << "\n";
}
//------------------------------------------------------------------------------

Session::Session(tcp::socket&& socket,
    ssl::context& ctx,
    std::shared_ptr<std::string const> const& doc_root,
    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia): __stream(std::move(socket), ctx), __doc_root(doc_root)
{
    __sessions_marussia = sessions_marussia;
    __sessions_mqtt = sessions_mqtt;
}

void Session::run() {
    net::dispatch( __stream.get_executor(), 
                    beast::bind_front_handler( &Session::__onRun, shared_from_this() ));
}
void Session::__onRun() {
    // Set the timeout.
    beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(30));
    // Perform the SSL handshake
    __stream.async_handshake( ssl::stream_base::server, 
                              beast::bind_front_handler( &Session::__onHandshake, this->shared_from_this() ));
}
void Session::__onHandshake(beast::error_code ec) {
    if (ec) { return fail(ec, "handshake"); }
    __is_live = true;
    __doRead();
}
void Session::__doRead() {
    __req = {};

    // Set the timeout.
    beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(__stream, __buffer, __req,
                    beast::bind_front_handler( &Session::__onRead, this->shared_from_this() ));
}
void Session::__onRead(beast::error_code ec, std::size_t bytes_transferred) 
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if (ec == http::error::end_of_stream) { return __doClose(); }

    if (ec) { return fail(ec, "read"); }

    // Send the response
    __sendResponse(__handleRequest());
}
void Session::__sendResponse(http::message_generator&& msg) {
    bool keep_alive = msg.keep_alive();

    // Write the response
    beast::async_write( __stream, std::move(msg),
                        beast::bind_front_handler( &Session::__onWrite, this->shared_from_this(), keep_alive));
}
void Session::__onWrite(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) { return fail(ec, "write"); }
        

    if (!keep_alive)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return __doClose();
    }

    // Read another request
    __doRead();
}
void Session::__doClose() {
    // Set the timeout.
    beast::get_lowest_layer(__stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    __stream.async_shutdown(
        beast::bind_front_handler(
            &Session::__onShutdown,
            shared_from_this()));
}
void Session::__onShutdown(beast::error_code ec) {
    if (ec) { return fail(ec, "shutdown"); }
    __is_live = false;
    // At this point the connection is closed gracefully
}
bool Session::isLive() {
    return __is_live;
}

//------------------------------------------------------------------------------

Listener::Listener( net::io_context& ioc,
                    ssl::context& ctx,
                    tcp::endpoint endpoint,
                    std::shared_ptr<std::string const> const& doc_root, 
                    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
                    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia): 
                                                                         __ioc(ioc),
                                                                         __ctx(ctx),
                                                                         __acceptor(ioc),
                                                                         __doc_root(doc_root)
{
    beast::error_code ec;

    // Open the acceptor
    __acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    __acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    __acceptor.bind(endpoint, ec);
    if (ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    __acceptor.listen(
        net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        fail(ec, "listen");
        return;
    }

    __sessions_mqtt = sessions_mqtt;
    __sessions_marussia = sessions_marussia;
    __timer_kill = std::make_shared<boost::asio::deadline_timer>(ioc);
    __sessions = std::make_shared<std::vector<std::shared_ptr<HTTPS_Server::Session>>>();
}
void Listener::run() {
    __acceptor.async_accept(net::make_strand(__ioc), beast::bind_front_handler(&Listener::__onAccept, shared_from_this()));
    __timer_kill->expires_from_now(boost::posix_time::seconds(__TIME_DEAD_SESSION));
    __timer_kill->async_wait(beast::bind_front_handler(&Listener::__killSession,shared_from_this()));
}
void Listener::__killSession(beast::error_code ec) {
    bool kill = false;
    for (auto i = __sessions->begin(); i != __sessions->end(); i++) {
        if (!(*i)->isLive()) {
            __sessions->erase(i);
            kill = true;
            break;
        }
    }
    if (kill) {
        __timer_kill->expires_from_now(boost::posix_time::seconds(0));
        __timer_kill->async_wait(beast::bind_front_handler(&Listener::__killSession, shared_from_this()));
    }
    else {
        __timer_kill->expires_from_now(boost::posix_time::seconds(__TIME_DEAD_SESSION));
        __timer_kill->async_wait(beast::bind_front_handler(&Listener::__killSession, shared_from_this()));
    }
}
void Listener::__onAccept(beast::error_code ec, tcp::socket socket) {
    if (ec)
    {
        fail(ec, "accept");
        return; // To avoid infinite loop
    }
    else
    {
        // Create the session and run it
        __sessions->push_back(std::make_shared<HTTPS_Server::Session>(
                                std::move(socket),
                                __ctx,
                                __doc_root,
                                __sessions_mqtt, __sessions_marussia));
        __sessions->back()->run();
    }

    // Accept another connection
    __acceptor.async_accept(net::make_strand(__ioc), beast::bind_front_handler(&Listener::__onAccept, shared_from_this()));
}

http::message_generator Session::__handleRequest()
{
    std::cout << std::endl << __req.method() << std::endl << std::endl;
    if (__req.method() != http::verb::post && __req.method() != http::verb::options) {
        __req = {};
        return __badRequest("Method not equal OPTIONS OR POST\n");
    }

    if (__req.method() == http::verb::options) {
        beast::string_view response_data = "POST, OPTIONS";
        http::response<http::string_body> res{ std::piecewise_construct,
            std::make_tuple(""),
            std::make_tuple(http::status::ok, __req.version()) };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_methods, response_data.data());
        res.set(http::field::access_control_allow_headers, "Content-Type, Accept");
        //res.set(http::field::content_length, "0");
        res.prepare_payload();
        res.keep_alive(true);
        __req = {};
        return res;
    }

    //POST
    beast::error_code err_code;
    json::value body_request;
    json::object body_response;

    std::cout << "REQUEST" << std::endl;
    std::cout << "Base: " << __req.base() << std::endl;
    std::cout << "Body: " << __req.body() << std::endl;
    
    try {
        __parser.reset();
        __parser.write(__req.body(), err_code);
        if (err_code) {
            __req = {};
            return __badRequest("parse JSON error");
        }
        if (!__parser.done()) {
            __req = {};
            return __badRequest("JSON not full");
        }
    }
    catch (std::bad_alloc const& e) {
        __req = {};
        return __badRequest(std::string("Bad alloc JSON: ") + e.what());
    }
    body_request = __parser.release();
    
    //проверки кому отправить

    //-----------------------

    //тест

    body_response["response"] =
    {
        {"text", std::string("eto ") + body_request.at("request").at("original_utterance").as_string().c_str()},
        {"tts", "AAAA"},
        //{"buttons", {}},
        {"end_session", false},
        //{"card", {}},
        //{"commands", {}}
    };
    body_response["session"] =
    {
        {"session_id", body_request.at("session").at("session_id")},
        {"user_id", body_request.at("session").at("application").at("application_id")},
        {"message_id", body_request.at("session").at("message_id").as_int64()}
    };
    body_response["version"] = {"1.0"};
    
    // Respond to POST request
    http::response<http::string_body> res{
        std::piecewise_construct,
            //std::make_tuple(std::move(body_response.)),
            std::make_tuple(""),
            std::make_tuple(http::status::ok, __req.version()) };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json;charset=utf-8");
    res.set(http::field::access_control_allow_origin, "*");
    //res.content_length(.size());
    
    res.keep_alive(__req.keep_alive());
    res.body() = json::serialize(body_response);
    res.prepare_payload();
    std::cout << "RESPONSE" << std::endl;
    std::cout << "Base: " << res.base() << std::endl;
    std::cout << "Body: " << res.body() << std::endl;

    __req = {};
    return res;
}
http::message_generator Session::__badRequest(beast::string_view why) {
    std::cout << "ERROR " << why << std::endl;
    http::response<http::string_body> res{http::status::bad_request, __req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json;charset=utf-8");
    res.keep_alive(__req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
}