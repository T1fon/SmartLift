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

void Server::start(std::shared_ptr<shared_ptr<map<string, vector<string>>>> sp_db_worker_ids) {
    __sp_db_worker_ids = sp_db_worker_ids;
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

                __sessions->push_back(std::make_shared<SessionMQTT>(__sender,socket,__sp_db_worker_ids, boost::asio::deadline_timer (*__context), boost::asio::deadline_timer(*__context)));
                break;
            case WORKER_MARUSIA_T:
                __sessions->push_back(std::make_shared<SessionMarusia>(__sender, socket,__sp_db_worker_ids, boost::asio::deadline_timer(*__context), boost::asio::deadline_timer(*__context)));
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


Session::Session(string sender, boost::asio::ip::tcp::socket& socket, std::shared_ptr<shared_ptr<map<string, vector<string>>>> sp_db_worker_ids,
    boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer) :
    _socket(std::move(socket)),
    _ping_timer(std::move(ping_timer)), _dead_ping_timer(std::move(dead_ping_timer))
{
    _sp_db_worker_ids = sp_db_worker_ids;
    _callback = boost::bind(&Session::__emptyCallback, this, _1);
    _sender = sender;
}
Session::~Session() {
    this->stop();
}
void Session::start() {
    _socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE), boost::bind(&Session::_reciveCommand, shared_from_this(), _1, _2));
}
void Session::stop() {
    _is_live = false;
    _ping_timer.cancel();
    _dead_ping_timer.cancel();
    if (_socket.is_open()) {
        _socket.close();
    }
}
void Session::_autorization() {
    boost::json::value analize_value = _buf_json_recive.front();
    _buf_json_recive.pop();
    try {
        _id = "\"" + string(analize_value.at("request").at("id").as_string().c_str()) + "\"";
        bool successful_find = false;

        /*��������� ���� �� ����� ������������*/
        string worker_name_id = "Id";

        for (auto i = (*_sp_db_worker_ids)->at(worker_name_id).begin(), end = (*_sp_db_worker_ids)->at(worker_name_id).end(); i != end; i++) {
            if (_id == (*i)) {
                successful_find = true;
                break;
            }
        }
        if (!successful_find) {
            throw invalid_argument("id not Found");
            //throw exception("id not Found");
        }
        /*------------------------------------*/
        //� ������ ������
        _is_live = true;
        _buf_send = serialize(json_formatter::worker::response::connect(_sender));

        _ping_timer.expires_from_now(boost::posix_time::seconds(ISession::_PING_TIME));
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
    size_t count_byte, count_byte_write = 0;
    for (; count_byte_write < count_recive_byte;) {
        try {
            count_byte = _json_parser.write_some(_buf_recive + count_byte_write);
        }
        catch (exception& e) {
            cerr << e.what() << endl;
        }

        if (!_json_parser.done()) {
            fill_n(_buf_recive, _BUF_RECIVE_SIZE, 0);
            cerr << "_reciveCheck json not full " << endl;
            _socket.async_receive(boost::asio::buffer(_buf_recive, _BUF_RECIVE_SIZE), handler);
            return _CHECK_STATUS::FAIL;
        }
        try {
            _buf_json_recive.push(_json_parser.release());
            _json_parser.reset();
            count_byte_write += count_byte;
        }
        catch (exception& e) {
            _json_parser.reset();
            _buf_json_recive = {};
            cerr << "_reciveCheck " << e.what();
            return _CHECK_STATUS::FAIL;
        }
    }
    fill_n(_buf_recive, _BUF_RECIVE_SIZE, 0);
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
    if (!_is_live) {
        this->stop();
        return;
    }
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
    _next_recive = true;
    _ping_success = false;
    _dead_ping_timer.cancel();
    _dead_ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME*2));
    _dead_ping_timer.async_wait(boost::bind(&Session::_deadPing, shared_from_this(), _1));

    _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
        boost::bind(&Session::_sendCommand, shared_from_this(), _1, _2));
}
void Session::_analizePing()
{
    boost::json::value analise_value = _buf_json_recive.front();
    _buf_json_recive.pop();
    try {
        if (analise_value.at("response").at("status") == "success") {
            _ping_success = true;
            _dead_ping_timer.cancel();
            _ping_timer.expires_from_now(boost::posix_time::seconds(_PING_TIME));
            _ping_timer.async_wait(boost::bind(&Session::_ping, shared_from_this(), _1));
        }
        else {
            _is_live = false;
            //cerr << "_analizePing Error response status not equal success, status = " << analise_value.at("response").at("status") << endl;
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
    if (!_ping_success) {
        //_dead_ping_timer.cancel();
        _is_live = false;

        if (error) {
            cerr << error << endl;
        }

        this->stop();
    }
}
bool Session::isLive() {
    return _is_live;
}
void Session::__emptyCallback(boost::json::value data)
{
    cerr << "required __emptyCallback" << endl;
}
void Session::_commandAnalize() {}
void Session::startCommand(COMMAND_CODE_MQTT command_code, void* command_parametr, _callback_t callback) {}
void Session::startCommand(COMMAND_CODE_MARUSIA command_code, void* command_parametr, _callback_t callback) {}
string Session::getId(){
    return _id;
}
//-------------------------------------------------------------//
SessionMQTT::SessionMQTT(string sender, boost::asio::ip::tcp::socket& socket, std::shared_ptr<shared_ptr<map<string, vector<string>>>> sp_db_worker_lu, boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer) :
    Session(sender, socket,sp_db_worker_lu, std::move(ping_timer), std::move(dead_ping_timer))
{}
SessionMQTT::~SessionMQTT() {
    this->stop();
}
void SessionMQTT::_commandAnalize() {
    for (; _buf_json_recive.size() > 0;) {
        try {
            boost::json::value target = _buf_json_recive.front().at("target");
            if (target == "ping") {
                _analizePing();
            }
            else if (target == "mqtt_message") {
                /**/
                __mqttMessage();
            }
            else if (target == "connect") {
                _autorization();
            }
        }
        catch (exception& e) {
            cerr << "_commandAnalize " << e.what() << endl;
        }
    }
}
void SessionMQTT::startCommand(COMMAND_CODE_MQTT command_code, void* command_parametr, _callback_t callback) {
    if (command_code == COMMAND_CODE_MQTT::MOVE_LIFT) {
        _callback = callback;
        move_lift_t* parametr = (move_lift_t*)command_parametr;
        _buf_send = serialize(json_formatter::worker::request::mqtt_move(_sender, parametr->station_id, parametr->lift_block_id, parametr->floor));
        _next_recive = true;
        _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()),
            boost::bind(&SessionMQTT::_sendCommand, shared_from_this(), _1, _2));
    }
}
void SessionMQTT::__mqttMessage() {
    _callback(_buf_json_recive.front());
    _buf_json_recive.pop();
}

//-------------------------------------------------------------//

SessionMarusia::SessionMarusia(string sender, boost::asio::ip::tcp::socket& socket, std::shared_ptr<shared_ptr<map<string, vector<string>>>> sp_db_worker_marusia,
    boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer) :
    Session(sender, socket,sp_db_worker_marusia, std::move(ping_timer), std::move(dead_ping_timer))
{}
SessionMarusia::~SessionMarusia() {
    this->stop();
}

void SessionMarusia::_commandAnalize() 
{
    try {
        boost::json::value target = _buf_json_recive.front().at("target");
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
void SessionMarusia::startCommand(COMMAND_CODE_MARUSIA command_code, void* command_parametr, _callback_t callback)
{
    if (command_code == MARUSIA_STATION_REQUEST) {
        _callback = callback;
        marussia_station_request_t* parametr = (marussia_station_request_t*)command_parametr;
        _buf_send = serialize(json_formatter::worker::request::marussia_request(_sender, parametr->station_id, parametr->body));
        _next_recive = true;
        _socket.async_send(boost::asio::buffer(_buf_send, _buf_send.size()), 
                            boost::bind(&SessionMarusia::_sendCommand, shared_from_this(), _1, _2));
    }
}
void SessionMarusia::__staticMessage() 
{
    _callback(_buf_json_recive.front());
    _buf_json_recive.pop();
}
void SessionMarusia::__moveLift() 
{
    _callback(_buf_json_recive.front());
    _buf_json_recive.pop();
}

