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
        enum _CHECK_STATUS {
            SUCCESS = 1,
            FAIL
        };
        typedef std::function<void(boost::system::error_code, std::size_t)> _handler_t;

        static const int _BUF_RECIVE_SIZE = 2048;
        static const int _PING_TIME = 5;
        string _id;
        string _sender;
        string _buf_send;
        char *_buf_recive;
        boost::json::stream_parser _json_parser;
        boost::json::value _buf_json_recive;
        bool _next_recive = false;
        bool _is_live = false;
        
        virtual void _autorization() = 0;
        virtual _CHECK_STATUS _reciveCheck(const size_t& count_recive_byte, _handler_t&& handler) = 0;
        virtual _CHECK_STATUS _sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, _handler_t&& handler) = 0;
        virtual void _sendCommand(const boost::system::error_code& error, std::size_t count_send_byte) = 0;
        virtual void _reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte) = 0;
        virtual void _commandAnalize() = 0;
        virtual void _ping(const boost::system::error_code& error) = 0;
        virtual void _analizePing() = 0;
        virtual void _deadPing(const boost::system::error_code& error) = 0;
        
    public:
        ISession();
        virtual ~ISession();
        
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isLive() = 0;
    };
    
    class SessionMQTT : public std::enable_shared_from_this<SessionMQTT>, public ISession {
    public:
        enum COMMAND_CODE {
            MOVE_LIFT = 1,
        };
        struct param_move_lift_t {
            std::string station_id = "";
            std::string lift_block_id = "";
            int floor = 0;
        };
    private:
        boost::asio::ip::tcp::socket __socket;
        boost::asio::deadline_timer __ping_timer;
        boost::asio::deadline_timer __dead_ping_timer;

        virtual void _autorization() override;
        virtual _CHECK_STATUS _reciveCheck(const size_t& count_recive_byte, _handler_t&& handler) override;
        virtual _CHECK_STATUS _sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, _handler_t&& handler) override;
        virtual void _sendCommand(const boost::system::error_code& error, std::size_t count_send_byte) override;
        virtual void _reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte) override;
        virtual void _commandAnalize() override;
        virtual void _ping(const boost::system::error_code& error) override;
        virtual void _analizePing() override;
        virtual void _deadPing(const boost::system::error_code& error) override;
        
        void __startCommand(SessionMQTT::COMMAND_CODE command_code, void *command_parametr);

    public:
        
        SessionMQTT(string sender, boost::asio::ip::tcp::socket &socket, boost::asio::deadline_timer ping_timer, boost::asio::deadline_timer dead_ping_timer);
        virtual ~SessionMQTT();
        
        virtual void start() override;
        virtual void stop() override;
        virtual bool isLive() override;
    };
    class SessionMarussia : public std::enable_shared_from_this<SessionMarussia>, public ISession {
    private:
        boost::asio::ip::tcp::socket __socket;
        
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
        std::shared_ptr <boost::asio::io_context> __context;
        std::vector<std::shared_ptr<ISession>> __sessions;
        
        void __doAccept();
    public:
        Server(std::shared_ptr < boost::asio::io_context> io_context, unsigned short port, WORKER_T worker_type, string sender = "Main_server");
        ~Server();
        void start();
        void stop();
    };
}

