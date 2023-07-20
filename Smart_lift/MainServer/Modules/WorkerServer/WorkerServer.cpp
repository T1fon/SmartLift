#include "WorkerServer.hpp"

using namespace worker_server;

//-------------------------------------------------------------//




Server::Server(boost::asio::io_context& io_context, unsigned short port, WORKER_T worker_type,string sender){
    __worker_type = worker_type;
    __sender = sender;
	__acceptor = make_shared<boost::asio::ip::tcp::acceptor>(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
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
                __sessions.push_back(std::make_shared<SessionMQTT>(__sender,socket));
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
    __id = to_string(NO_ID);
    __buf_send = "";
    //__buf_recive = "";
    __buf_recive = new char[BUF_RECIVE_SIZE+1];
    fill_n(__buf_recive,BUF_RECIVE_SIZE, 0);
    __sender = "";
    __json_parser.reset();
}
ISession::~ISession() { cout << "ISESSION DELETE" << endl; delete[] __buf_recive; }

//-------------------------------------------------------------//

SessionMQTT::SessionMQTT(string sender,  boost::asio::ip::tcp::socket &socket): __socket(std::move(socket)) { __sender = sender; }
SessionMQTT::~SessionMQTT(){
    this->stop();
}
void SessionMQTT::start() {
    __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&SessionMQTT::__requestAutorization, shared_from_this(), _1, _2));
    
}
void SessionMQTT::stop() {
    if (__socket.is_open()) {
        __socket.close();
    }
}
void SessionMQTT::__requestAutorization(const boost::system::error_code& error, size_t count_recive_bytes) {
    if (error) {
        cerr << error.message() << endl;
        __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&SessionMQTT::__requestAutorization, shared_from_this(), _1, _2));
        return;
        
    }
        
    try {
        __json_parser.write(__buf_recive,count_recive_bytes);
    }
    catch (exception& e) {
        
        cerr << e.what() << endl;
    }
    
    fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
    if (!__json_parser.done()) {
        cerr << "connectAnalize json not full" << endl;
        __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
            boost::bind(&SessionMQTT::__requestAutorization, shared_from_this(), _1, _2));
        return;
    }

    __json_parser.finish();
    __buf_json_recive = __json_parser.release();
    __json_parser.reset();

    /*Проверяем есть ли такой пользователь чтобы отетить*/

    /**/
    try {
        if (__buf_json_recive.at("target").as_string() != "connect") {
            __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
                boost::bind(&SessionMQTT::__requestAutorization, shared_from_this(), _1, _2));
            return;
        }
        __id = __buf_json_recive.at("request").at("id").as_string();
        //*---
        __buf_send = serialize(json_formatter::worker::response::connect(__sender));
    }
    catch (exception &e) {
        cerr << e.what() << endl;
        __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
            boost::bind(&SessionMQTT::__requestAutorization, shared_from_this(), _1, _2));
        return;
    }

    
    __socket.async_send(boost::asio::buffer(__buf_send, __buf_send.size()), 
                     boost::bind(&SessionMQTT::__responseAutorization, shared_from_this(), _1, _2));
}
void SessionMQTT::__responseAutorization(const boost::system::error_code& error, size_t count_send_bytes) {
    if (error) {
        cerr << error.message() << endl;
        //чтобы не спамить временное решение
        Sleep(2000);
        __socket.async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
            boost::bind(&SessionMQTT::__responseAutorization, shared_from_this(), _1, _2));
        return;
    }

    static size_t temp_send_byte = 0;
    temp_send_byte += count_send_bytes;
    if (__buf_send.size() != temp_send_byte) {
        __socket.async_send(boost::asio::buffer(__buf_send.c_str() + temp_send_byte, (__buf_send.size() - temp_send_byte)),
            boost::bind(&SessionMQTT::__responseAutorization, shared_from_this(), _1, _2));
        return;
    }
    temp_send_byte = 0;
    __buf_send = "";
    //-----//
    __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
        boost::bind(&SessionMQTT::__responseAutorization, shared_from_this(), _1, _2));
}

//-------------------------------------------------------------//

SessionMarussia::SessionMarussia(string sender, boost::asio::ip::tcp::socket socket) : __socket(std::move(socket)) { __sender = sender; }
SessionMarussia::~SessionMarussia() {
    this->stop();
}
void SessionMarussia::start() {
    __socket.async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&SessionMarussia::__requestAutorization,shared_from_this(),_1,_2));
}
void SessionMarussia::stop() {
    if(__socket.is_open()) {
        __socket.close();
    }
}
void SessionMarussia::__requestAutorization(const boost::system::error_code& error, size_t count_recive_bytes) {
    if (error) {

    }
}
void SessionMarussia::__responseAutorization(const boost::system::error_code& error, size_t count_recive_bytes) {
}