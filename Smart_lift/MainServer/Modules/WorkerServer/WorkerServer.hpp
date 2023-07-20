#pragma once
#include <iostream>
#include <csignal>
#include <boost/bind.hpp>
#include <boost/lambda2.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "../Smart_Lift/Smart_lift/GlobalModules/JSONFormatter/JSONFormatter.hpp"

using namespace std;
typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

namespace worker_server {
    const short NO_ID = -1;
    enum WORKER_T {
        WORKER_MQTT_T = 1,
        WORKER_MARUSSIA_T
    };
    class ISession {
    protected:
        static const int BUF_RECIVE_SIZE = 2048;
        string __id;
        string __sender;
        string __buf_send;
        char *__buf_recive;
        boost::json::stream_parser __json_parser;
        boost::json::value __buf_json_recive;
        
        virtual void __requestAutorization(const boost::system::error_code& error, size_t count_recive_bytes) = 0;
        virtual void __responseAutorization(const boost::system::error_code& error, size_t count_send_bytes) = 0;

    public:
        ISession();
        virtual ~ISession();
        
        virtual void start() = 0;
        virtual void stop() = 0;
    };
    
    class SessionMQTT : public std::enable_shared_from_this<SessionMQTT>, public ISession {
    private:
        boost::asio::ip::tcp::socket __socket;
        virtual void __requestAutorization(const boost::system::error_code& error, size_t count_recive_bytes) override;
        virtual void __responseAutorization(const boost::system::error_code& error, size_t count_send_bytes) override;
    public:
        SessionMQTT(string sender, boost::asio::ip::tcp::socket &socket);
        virtual ~SessionMQTT();
        
        virtual void start() override;
        virtual void stop() override;
    };
    class SessionMarussia : public std::enable_shared_from_this<SessionMarussia>, public ISession {
    private:
        boost::asio::ip::tcp::socket __socket;
        virtual void __requestAutorization(const boost::system::error_code& error, size_t count_recive_bytes) override;
        virtual void __responseAutorization(const boost::system::error_code& error, size_t count_send_bytes) override;
    public:
        SessionMarussia(string sender, boost::asio::ip::tcp::socket socket);
        virtual ~SessionMarussia();
        virtual void start() override;
        virtual void stop() override;
    };

    
    class Server {
    private:
        WORKER_T __worker_type;
        string __sender;
        std::shared_ptr < boost::asio::ip::tcp::acceptor> __acceptor;
        std::vector<std::shared_ptr<ISession>> __sessions;
        void __doAccept();
    public:
        Server(boost::asio::io_context& io_context, unsigned short port, WORKER_T worker_type, string sender = "Main_server");
        ~Server();
        void start();
        void stop();
    };
}

