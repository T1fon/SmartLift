#include "WorkerServer.hpp"

using namespace worker_server;

//-------------------------------------------------------------//

Server::Server(std::shared_ptr < boost::asio::io_context> io_context, unsigned short port, WORKER_T worker_type,string sender){
    __worker_type = worker_type;
    __sender = sender;
    __context = io_context;
	__acceptor = make_shared<boost::asio::ip::tcp::acceptor>(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    __kill_timer = make_shared<boost::asio::deadline_timer>(*__context);
    __sessions = make_shared<std::vector<std::shared_ptr<Session>>>();
}

Server::~Server() {
	this->stop();
}

void Server::start() {
	__accept();
    __kill_timer->expires_from_now(boost::posix_time::seconds(__TIME_KILL));
    __kill_timer->async_wait(boost::bind(&Server::__killSession, shared_from_this(), _1));
}

void Server::stop() {
    for (size_t i = 0, length = __sessions->size(); i < length; i++) {
        __sessions->back()->stop();
    }
    __sessions->clear();
}

void Server::__accept() {
    __acceptor->async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
        {
            if (error) {
                cerr << error.message() << endl;
                __accept();
            }
            switch (__worker_type) {
            case WORKER_MQTT_T:

                __sessions->push_back(std::make_shared<SessionMQTT>(__sender,socket, boost::asio::deadline_timer (*__context), boost::asio::deadline_timer(*__context)));
                break;
            case WORKER_MARUSSIA_T:
                __sessions->push_back(std::make_shared<SessionMarussia>(__sender, socket, boost::asio::deadline_timer(*__context), boost::asio::deadline_timer(*__context)));
                break;
            }
            
            __sessions->back()->start();
            __accept();
        }
    );
}
void Server::__killSession(const boost::system::error_code& error) {
    bool kill = false;
    for (auto i = __sessions->begin(); i != __sessions->end(); i++) {
        if (!(*i)->isLive()) {
            (*i)->stop();
            __sessions->erase(i);
            kill = true;
            break;
        }
    }
    if (kill) {
        __kill_timer->expires_from_now(boost::posix_time::seconds(0));
        __kill_timer->async_wait(boost::bind(&Server::__killSession, shared_from_this(), _1));
    }
    else{
        __kill_timer->expires_from_now(boost::posix_time::seconds(__TIME_KILL));
        __kill_timer->async_wait(boost::bind(&Server::__killSession, shared_from_this(), _1));
    }
}
std::shared_ptr<std::vector<std::shared_ptr<Session>>> Server::getSessions() {
    return __sessions;
}
//-------------------------------------------------------------//

ISession::ISession(){
    _id = to_string(NO_ID);
    _buf_send = "";
    _buf_recive = new char[_BUF_RECIVE_SIZE+1];
    fill_n(_buf_recive,_BUF_RECIVE_SIZE, 0);
    _sender = "";
    _json_parser.reset();
}
ISession::~ISession() { cout << "ISESSION DELETE" << endl; delete[] _buf_recive; }

//-------------------------------------------------------------//


Session::Session(string sender, boost::asio::ip::tcp::socket& socket,
    boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer) :
    _socket(std::move(socket)),
    _ping_timer(std::move(ping_timer)), _dead_ping_timer(std::move(dead_ping_timer)), 
    _callback(boost::bind(&Session::__emptyCallback, shared_from_this(), _1, _2))
{
    _sender = sender;
}
Session::~Session() {
    this->stop();
}
void Session::start() {
    _socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE), boost::bind(&Session::_reciveCommand, shared_from_this(), _1, _2));
}
void Session::stop() {
    if (_socket.is_open()) {
        _is_live = false;
        _socket.close();
    }
}
void Session::_autorization() {
    try {
        _id = _buf_json_recive.at("request").at("id").as_string();
        cout << "ID: " << _id << endl;
        /*Проверить есть ли такой пользователь*/

        /*------------------------------------*/
        //В случае успеха
        _is_live = true;
        _buf_send = serialize(json_formatter::worker::response::connect(_sender));

        _ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME));
        _ping_timer.async_wait(boost::bind(&Session::_ping, shared_from_this(), _1));
    }
    catch (exception& e) {
        cerr << "__autorization " << e.what();
        _buf_send = serialize(json_formatter::worker::response::connect(_sender, json_formatter::ERROR_CODE::CONNECT, "Field Id not found"));
    }
    _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
        boost::bind(&Session::_sendCommand, shared_from_this(), _1, _2));
}
Session::_CHECK_STATUS Session::_reciveCheck(const size_t& count_recive_byte, _handler_t&& handler)
{
    try {
        _json_parser.write(_buf_recive, count_recive_byte);
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }

    fill_n(_buf_recive, _BUF_RECIVE_SIZE, 0);

    if (!_json_parser.done()) {
        cerr << "_reciveCheck json not full " << endl;
        _socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE), handler);
        return _CHECK_STATUS::FAIL;
    }
    try {
        _json_parser.finish();
        _buf_json_recive = _json_parser.release();
        _json_parser.reset();
    }
    catch (exception& e) {
        _json_parser.reset();
        _buf_json_recive = {};
        cerr << "_reciveCheck " << e.what();
        return _CHECK_STATUS::FAIL;
    }
    return _CHECK_STATUS::SUCCESS;
}
Session::_CHECK_STATUS Session::_sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, _handler_t&& handler)
{
    temp_send_byte += count_send_byte;
    if (_buf_send.size() != temp_send_byte) {
        _socket.async_send(boost::asio::buffer(_buf_send.c_str() + temp_send_byte, (_buf_send.size() - temp_send_byte)), handler);
        return _CHECK_STATUS::FAIL;
    }
    return _CHECK_STATUS::SUCCESS;
}
void Session::_sendCommand(const boost::system::error_code& error, std::size_t count_send_byte)
{
    if (error) {
        cerr << "sendCommand " << error.what() << endl;
        this->stop();
        return;
    }

    static size_t temp_send_byte = 0;
    if (_sendCheck(count_send_byte, temp_send_byte, boost::bind(&Session::_sendCommand, shared_from_this(), _1, _2)) == _CHECK_STATUS::FAIL) {
        return;
    }
    temp_send_byte = 0;
    _buf_send = "";
    if (_next_recive) {
        _next_recive = false;
        _socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE),
            boost::bind(&Session::_reciveCommand, shared_from_this(), _1, _2));
    }
}
void Session::_reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte)
{
    if (error) {
        cerr << "_reciveCommand " << error.what() << endl;
        this->stop();
        return;
    }

    if (_reciveCheck(count_recive_byte, boost::bind(&Session::_reciveCommand, shared_from_this(), _1, _2)) == _CHECK_STATUS::FAIL)
    {
        return;
    }

    _commandAnalize();
    _buf_json_recive = {};
}
void Session::_ping(const boost::system::error_code& error)
{
    if (error) {
        cerr << "_ping " << error.what() << endl;
        this->stop();
        return;
    }
    _buf_send = serialize(json_formatter::worker::request::ping(_sender));
    cout << _buf_send << endl;
    _next_recive = true;

    _dead_ping_timer.cancel();
    _dead_ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME * 2));
    _dead_ping_timer.async_wait(boost::bind(&Session::_deadPing, shared_from_this(), _1));

    _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
        boost::bind(&Session::_sendCommand, shared_from_this(), _1, _2));
}
void Session::_analizePing()
{
    try {
        cout << _buf_json_recive << endl;
        if (_buf_json_recive.at("response").at("status") == "success") {
            _ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME));
            _ping_timer.async_wait(boost::bind(&Session::_ping, shared_from_this(), _1));
        }
        else {
            _is_live = false;
            cerr << "_analizePing Error response status not equal success, status = " << _buf_json_recive.at("response").at("status") << endl;
            this->stop();
            return;
        }
    }
    catch (exception& e) {
        cerr << "_analizePing " << e.what() << endl << "Session stop" << endl;
        this->stop();
        return;
    }
}
void Session::_deadPing(const boost::system::error_code& error) {
    if (!_is_live) {
        _dead_ping_timer.cancel();
        cout << "DEAD PING" << endl;
        if (error) {
            cerr << error << endl;
            cerr << error.what() << " " << error.message() << endl;
        }

        this->stop();
    }
}
bool Session::isLive() {
    return _is_live;
}
void Session::__emptyCallback(boost::system::error_code error, boost::json::value data)
{
    cerr << "Был вызван __emptyCallback" << endl;
}
void Session::_commandAnalize() {}
void Session::startCommand(COMMAND_CODE_MQTT command_code, void* command_parametr, _callback_t&& callback) {}
void Session::startCommand(COMMAND_CODE_MARUSSIA command_code, void* command_parametr, _callback_t&& callback) {}
//-------------------------------------------------------------//
SessionMQTT::SessionMQTT(string sender, boost::asio::ip::tcp::socket& socket, boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer) :
    Session(sender, socket, std::move(ping_timer), std::move(dead_ping_timer))
{}
SessionMQTT::~SessionMQTT() {
    this->stop();
}
void SessionMQTT::_commandAnalize() {
    try {
        boost::json::value target = _buf_json_recive.at("target");
        if (target == "ping") {
            _analizePing();
        }
        else if (target == "mqtt_message") {
            /**/
        }
        else if (target == "connect") {
            _autorization();
        }
    }
    catch (exception& e) {
        cerr << "_commandAnalize " << e.what() << endl;
    }
}
void SessionMQTT::startCommand(COMMAND_CODE_MQTT command_code, void* command_parametr, _callback_t&& callback) {
    if (command_code == COMMAND_CODE_MQTT::MOVE_LIFT) {
        _callback = callback;
        move_lift_t* parametr = (move_lift_t*)command_parametr;
        _buf_send = serialize(json_formatter::worker::request::mqtt_move(_sender, parametr->station_id, parametr->lift_block_id, parametr->floor));
        _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
            boost::bind(&SessionMQTT::_sendCommand, shared_from_this(), _1, _2));
    }
}
void SessionMQTT::__mqttMessage() {
    _callback({}, _buf_json_recive);
}

//-------------------------------------------------------------//

SessionMarussia::SessionMarussia(string sender, boost::asio::ip::tcp::socket& socket,
    boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer) :
    Session(sender, socket, std::move(ping_timer), std::move(dead_ping_timer))
{}
SessionMarussia::~SessionMarussia() {
    this->stop();
}

void SessionMarussia::_commandAnalize() 
{
    try {
        boost::json::value target = _buf_json_recive.at("target");
        if (target == "ping") {
            _analizePing();
        }
        else if (target == "static_message") {
            /**/
            __staticMessage();
        }
        else if (target == "move_lift") {
            /**/
            __moveLift();
        }
        else if (target == "connect") {
            _autorization();
        }
    }
    catch (exception& e) {
        cerr << "_commandAnalize " << e.what() << endl;
    }
}
void SessionMarussia::startCommand(COMMAND_CODE_MARUSSIA command_code, void* command_parametr, _callback_t&& callback)
{
    if (command_code == MARUSSIA_STATION_REQUEST) {
        _callback = callback;
        marussia_station_request_t* parametr = (marussia_station_request_t*)command_parametr;
        _buf_send = serialize(json_formatter::worker::request::marussia_request(_sender, parametr->station_id, parametr->body));
        _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()), 
                            boost::bind(&SessionMarussia::_sendCommand, shared_from_this(), _1, _2));
    }
}
void SessionMarussia::__staticMessage() 
{
    _callback({}, _buf_json_recive);
}
void SessionMarussia::__moveLift() 
{
    _callback({}, _buf_json_recive);
}

