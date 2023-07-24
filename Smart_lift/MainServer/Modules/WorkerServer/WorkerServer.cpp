#include "WorkerServer.hpp"

using namespace worker_server;

//-------------------------------------------------------------//

Server::Server(std::shared_ptr < boost::asio::io_context> io_context, unsigned short port, WORKER_T worker_type,string sender){
    __worker_type = worker_type;
    __sender = sender;
    __context = io_context;
	__acceptor = make_shared<boost::asio::ip::tcp::acceptor>(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
}

Server::~Server() {
	this->stop();
}

void Server::start() {
	__doAccept();
}

void Server::stop() {
    for (size_t i = 0, length = __sessions.size(); i < length; i++) {
        __sessions.back()->stop();
    }
}

void Server::__doAccept() {
    __acceptor->async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
        {
            if (error) {
                cerr << error.message() << endl;
                __doAccept();
            }
            switch (__worker_type) {
            case WORKER_MQTT_T:

                __sessions.push_back(std::make_shared<SessionMQTT>(__sender,socket, boost::asio::deadline_timer (*__context), boost::asio::deadline_timer(*__context)));
                break;
            case WORKER_MARUSSIA_T:
                //__sessions.push_back(std::make_shared<SessionMarussia>(std::move(socket)));
                break;
            }
            
            __sessions.back()->start();
            __doAccept();
        }
    );
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

SessionMQTT::SessionMQTT(string sender,  boost::asio::ip::tcp::socket &socket, boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer):
__socket(std::move(socket)), __ping_timer(std::move(ping_timer)), __dead_ping_timer(std::move(dead_ping_timer))
{ 
    _sender = sender;
}
SessionMQTT::~SessionMQTT(){
    this->stop();
}
void SessionMQTT::start() {
    __socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE), boost::bind(&SessionMQTT::_reciveCommand, shared_from_this(), _1, _2));
}
void SessionMQTT::stop() {
    if (__socket.is_open()) {
        _is_live = false;
        __socket.close();
    }
}
SessionMQTT::_CHECK_STATUS SessionMQTT::_reciveCheck(const size_t& count_recive_byte, _handler_t&& handler) {
    try {
        _json_parser.write(_buf_recive, count_recive_byte);
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }

    fill_n(_buf_recive, _BUF_RECIVE_SIZE, 0);

    if (!_json_parser.done()) {
        cerr << "_reciveCheck json not full " << endl;
        __socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE), handler);
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
SessionMQTT::_CHECK_STATUS SessionMQTT::_sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, _handler_t&& handler) {
    temp_send_byte += count_send_byte;
    if (_buf_send.size() != temp_send_byte) {
        __socket.async_send(boost::asio::buffer(_buf_send.c_str() + temp_send_byte, (_buf_send.size() - temp_send_byte)), handler);
        return _CHECK_STATUS::FAIL;
    }
    return _CHECK_STATUS::SUCCESS;
}
void SessionMQTT::_sendCommand(const boost::system::error_code& error, std::size_t count_send_byte) {
    if (error) {
        cerr << "sendCommand " << error.what() << endl;
        this->stop();
        return;
    }

    static size_t temp_send_byte = 0;
    if (_sendCheck(count_send_byte, temp_send_byte, boost::bind(&SessionMQTT::_sendCommand, shared_from_this(), _1, _2)) == _CHECK_STATUS::FAIL) {
        return;
    }
    temp_send_byte = 0;
    _buf_send = "";
    if (_next_recive) {
        _next_recive = false;
        __socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE),
            boost::bind(&SessionMQTT::_reciveCommand, shared_from_this(), _1, _2));
    }
}
void SessionMQTT::_reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte) {
    if (error) {
        cerr << "_reciveCommand " << error.what() << endl;
        this->stop();
        return;
    }

    if (_reciveCheck(count_recive_byte, boost::bind(&SessionMQTT::_reciveCommand, shared_from_this(), _1, _2)) == _CHECK_STATUS::FAIL)
    {
        return;
    }
    
    _commandAnalize();
    _buf_json_recive = {};
}


void SessionMQTT::_autorization() {
    try {
        _id = _buf_json_recive.at("request").at("id").as_string();
        /*Проверить есть ли такой пользователь*/

        /*------------------------------------*/
        //В случае успеха
        _is_live = true;
        _buf_send = serialize(json_formatter::worker::response::connect(_sender));
        
        __ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME));
        __ping_timer.async_wait(boost::bind(&SessionMQTT::_ping, shared_from_this(), _1));
    }
    catch (exception& e) {
        cerr << "__autorization " << e.what();
        _buf_send = serialize(json_formatter::worker::response::connect(_sender, json_formatter::ERROR_CODE::CONNECT, "Field Id not found"));
    }
    __socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
        boost::bind(&SessionMQTT::_sendCommand, shared_from_this(), _1, _2));
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
        cerr << "_commandAnalize " << e.what() <<endl;
    }
}
void SessionMQTT::__startCommand(SessionMQTT::COMMAND_CODE command_code, void *command_parametr) {
    
    if(command_code == COMMAND_CODE::MOVE_LIFT){
        param_move_lift_t *parametr = (param_move_lift_t*)command_parametr;
        _buf_send = serialize(json_formatter::worker::request::mqtt_move(_sender,parametr->station_id,parametr->lift_block_id,parametr->floor));
        __socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
            boost::bind(&SessionMQTT::_sendCommand, shared_from_this(), _1, _2));
    }
}
void SessionMQTT::_ping(const boost::system::error_code& error) {
    if (error) {
        cerr << "_ping " << error.what() << endl;
        this->stop();
        return;
    }
    _buf_send = serialize(json_formatter::worker::request::ping(_sender));
    cout << _buf_send << endl;
    _next_recive = true;

    __dead_ping_timer.cancel();
    __dead_ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME * 2));
    __dead_ping_timer.async_wait(boost::bind(&SessionMQTT::_deadPing, shared_from_this(), _1));

    __socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
        boost::bind(&SessionMQTT::_sendCommand, shared_from_this(), _1, _2));
}
void SessionMQTT::_analizePing() {
    try {
        cout << _buf_json_recive << endl;
        if (_buf_json_recive.at("response").at("status") == "success") {
            __ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME));
            __ping_timer.async_wait(boost::bind(&SessionMQTT::_ping, shared_from_this(), _1));
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
void SessionMQTT::_deadPing(const boost::system::error_code& error) {
    
    if (!_is_live) {
        __dead_ping_timer.cancel();
        cout << "DEAD PING" << endl;
        if (error) {
            cerr << error << endl;
            cerr << error.what() << " " << error.message() << endl;
        }

        this->stop();
    }
}
bool SessionMQTT::isLive() {
    return _is_live;
}
//-------------------------------------------------------------//

SessionMarussia::SessionMarussia(string sender, boost::asio::ip::tcp::socket socket) : __socket(std::move(socket)) { _sender = sender; }
SessionMarussia::~SessionMarussia() {
    this->stop();
}
void SessionMarussia::start() {
//    __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&SessionMarussia::__requestAutorization,shared_from_this(),_1,_2));
}
void SessionMarussia::stop() {
    if(__socket.is_open()) {
        __socket.close();
    }
}