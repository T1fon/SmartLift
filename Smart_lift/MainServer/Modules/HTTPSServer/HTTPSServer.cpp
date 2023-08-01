#include "HTTPSServer.hpp"

using namespace https_server;

//------------------------------------------------------------------------------
void https_server::fail(beast::error_code ec, char const* what) {
    if (ec == net::ssl::error::stream_truncated) { return; }

    std::cerr << what << ": " << ec.message() << "\n";
}
//------------------------------------------------------------------------------

Session::Session(tcp::socket&& socket,
    ssl::context& ssl_ctx,
    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia): __stream(std::move(socket), ssl_ctx)
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
    //__sendResponse(__analizeRequest());
    __analizeRequest();
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

Listener::Listener( net::io_context& io_ctx,
                    ssl::context& ssl_ctx,
                    unsigned short port,
                    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt,
                    std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_marussia): 
                                                                         __io_ctx(io_ctx),
                                                                         __ssl_ctx(ssl_ctx)
{
    __acceptor = make_shared<tcp::acceptor>(__io_ctx, tcp::endpoint(tcp::v4(), port));

    __sessions_mqtt = sessions_mqtt;
    __sessions_marussia = sessions_marussia;
    __timer_kill = std::make_shared<boost::asio::deadline_timer>(io_ctx);
    __sessions = std::make_shared<std::vector<std::shared_ptr<https_server::Session>>>();
}
void Listener::start() {
    __acceptor->async_accept(beast::bind_front_handler(&Listener::__onAccept, shared_from_this()));
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
        __sessions->push_back(std::make_shared<https_server::Session>(
                                std::move(socket),
                                __ssl_ctx,
                                __sessions_mqtt, __sessions_marussia));
        __sessions->back()->run();
    }

    // Accept another connection
    __acceptor->async_accept(net::make_strand(__io_ctx), beast::bind_front_handler(&Listener::__onAccept, shared_from_this()));
}
void Session::__analizeRequest()
{
    std::cout << std::endl << __req.method() << std::endl << std::endl;
    if (__req.method() != http::verb::post && __req.method() != http::verb::options) {
        __req = {};
        __sendResponse(__badRequest("Method not equal OPTIONS OR POST\n"));
        return;
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
        __sendResponse((http::message<false,http::string_body,http::fields>)res);
        return;
    }

    //POST
    beast::error_code err_code;
    
    std::cout << "REQUEST" << std::endl;
    std::cout << "Base: " << __req.base() << std::endl;
    std::cout << "Body: " << __req.body() << std::endl;
    
    try {
        __parser.reset();
        __parser.write(__req.body(), err_code);
        if (err_code) {
            __req = {};
            __sendResponse(__badRequest("parse JSON error"));
            return;
        }
        if (!__parser.done()) {
            __req = {};
            __sendResponse(__badRequest("JSON not full"));
            return;
        }
    }
    catch (std::bad_alloc const& e) {
        __req = {};
        __sendResponse(__badRequest(std::string("Bad alloc JSON: ") + e.what()));
        return;
    }
    __body_request = __parser.release();
    
    //проверки кому отправить
    if (__sessions_marussia->size() == 0) {
        cerr << "__session_marussia size = 0" << endl;
        __callbackWorkerMarussia({}, {});
        return;
    }
    cout << "size " << __sessions_marussia->size() << endl;
    __sessions_marussia->at(0)->startCommand(worker_server::Session::COMMAND_CODE_MARUSSIA::MARUSSIA_STATION_REQUEST, (void*)&__body_request, 
                                            boost::bind(&Session::__callbackWorkerMarussia, shared_from_this(), _1, _2));
    //-----------------------

    //тест
    return;
   /* __body_response["response"] =
    {
        {"text", std::string("eto ") + __body_request.at("request").at("original_utterance").as_string().c_str()},
        {"tts", "AAAA"},
        //{"buttons", {}},
        {"end_session", false},
        //{"card", {}},
        //{"commands", {}}
    };
    __body_response["session"] =
    {
        {"session_id", __body_request.at("session").at("session_id")},
        {"user_id", __body_request.at("session").at("application").at("application_id")},
        {"message_id", __body_request.at("session").at("message_id").as_int64()}
    };
    __body_response["version"] = {"1.0"};
    
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
    res.body() = json::serialize(__body_response);
    res.prepare_payload();
    std::cout << "RESPONSE" << std::endl;
    std::cout << "Base: " << res.base() << std::endl;
    std::cout << "Body: " << res.body() << std::endl;

    __req = {};*/
    
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
void Session::__callbackWorkerMarussia(boost::system::error_code error, boost::json::value data) {
    boost:json::value target;
    boost::json::object response_data = {};
    boost::locale::generator gen;
    cout << "__callbackWorkerMarussia : " << data << endl;

    try {
        target = data.at("target");
    }
    catch (exception& e) {
        cout << "__callbackWorkerMarussia [target]: " << e.what();
        target = "error";
    };
    /*------------*/
    try {
        if (target == "static_message") {
            response_data = data.at("response_body").as_object();
        }
        else if (target == "move_lift") {
            if (__sessions_mqtt->size() == 0) {
                throw exception("session mqtt size = 0");
            }
            __sessions_mqtt->at(0)->startCommand(worker_server::Session::COMMAND_CODE_MQTT::MOVE_LIFT, (void*)&__body_request,
                boost::bind(&Session::__callbackWorkerMQTT, shared_from_this(), _1, _2));
            return;
        }
        else {
            target = "error";
        }
    }
    catch (exception& e) {
        cerr << "__callbackWorkerMarussia [target analize]: " << e.what() << endl;
        target = "error";
    };
    /*------------*/
    if(target == "error"){
        /*ошибка обработки запроса сервер временно недоступен*/
        /*стандартное сообщение о том что сервер временно недоступен*/
        u8string text = u8"—ервер временно недоступен, приношу свои извинени€, € работаю над устранением проблемы";
        string result_text = boost::locale::conv::to_utf<char>(string(text.begin(), text.end()), gen(""));
        response_data =
        {
            {"text", result_text},
            {"tts", result_text},
            {"end_session", true}, //mb ne nado zakrivat
        };
    }

    __constructResponse(response_data);
}
void Session::__callbackWorkerMQTT(boost::system::error_code error, boost::json::value data) {
    boost:json::value target;
    boost::json::object response_data = {};
    boost::locale::generator gen;
    u8string text;
    string result_text;
    cout << "__callbackWorkerMQTT : " << data << endl;

    try {
        target = data.at("target");
    }
    catch (exception& e) {
        cout << "__callbackWorkerMarussia [target]: " << e.what();
        target = "error";
    };

    /*------------*/
    try {
        if (target == "mqtt_message") {
            if (data.at("response").at("status") == "success") {
                text = u8"„ем ещЄ € могу вам помочь?";
                result_text = boost::locale::conv::to_utf<char>(string(text.begin(), text.end()), gen(""));
                
            }
            else {
                text = u8"ѕриношу свои извинени€, утер€на св€зь с лифтом";
                result_text = boost::locale::conv::to_utf<char>(string(text.begin(), text.end()), gen(""));
            }   
            response_data =
            {
                {"text", result_text},
                {"tts", result_text},
                {"end_session", false}, //mb ne nado zakrivat
            };
        }
        else {
            target = "error";
        }
    }
    catch (exception& e) {
        cerr << "__callbackWorkerMarussia [target analize]: " << e.what() << endl;
        target = "error";
    };
    //string_body a;
    
    /*------------*/
    if (target == "error") {
        /*ошибка обработки запроса сервер временно недоступен*/
        /*стандартное сообщение о том что сервер временно недоступен*/
        text = u8"—ервер временно недоступен, приношу свои извинени€, € работаем над устранением проблемы";
        result_text = boost::locale::conv::to_utf<char>(string(text.begin(), text.end()), gen(""));
        response_data =
        {
            {"text", result_text},
            {"tts", result_text},
            {"end_session", true}, //mb ne nado zakrivat
        };
    }

    __constructResponse(response_data);
}
void Session::__constructResponse(boost::json::object response_data) {

    __body_response["response"] = response_data;

    __body_response["session"] =
    {
        {"session_id", __body_request.at("session").at("session_id")},
        {"user_id", __body_request.at("session").at("application").at("application_id")},
        {"message_id", __body_request.at("session").at("message_id").as_int64()}
    };
    __body_response["version"] = { "1.0" };

    http::response<http::string_body> response{
        std::piecewise_construct,
            std::make_tuple(""),
            std::make_tuple(http::status::ok, __req.version()) };
    response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(http::field::content_type, "application/json; charset=utf-8; locale=ru-RU;");
    response.set(http::field::access_control_allow_origin, "*");
    response.keep_alive(__req.keep_alive());
    response.body() = serialize(__body_response);
    response.prepare_payload();
    __req = {};
    __body_response = {};
    __body_request = {};

    __sendResponse((http::message<false, http::string_body, http::fields>)response);
}