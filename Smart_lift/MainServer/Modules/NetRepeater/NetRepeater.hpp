#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <map>
#include <shared_mutex>


using namespace std;

namespace net_repeater {

	/*------------------------------------------------------------------------------*/
	
	class Session: public std::enable_shared_from_this<Session>
	{
	private:
		typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
		enum __CHECK_STATUS {
			SUCCESS = 1,
			FAIL
		};
		enum __SOCKET {
			FIRST = 1,
			SECOND
		};
		static const int __BUF_RECIVE_SIZE = 2048;
		const int __TIME_KILL = 30;
		shared_ptr<boost::asio::ip::tcp::socket> __socket_first;
		shared_ptr<boost::asio::ip::tcp::socket> __socket_second;
		boost::asio::deadline_timer __dead_timer;
		bool __is_live;
		bool __its_connect;
		
		string __buf_send_first;
		string __buf_send_second;
		char* __buf_recive_first;
		char* __buf_recive_second;

		shared_ptr<shared_ptr<map<string, vector<string>>>> __lb_worker_data;
		__CHECK_STATUS __sendCheck(__SOCKET num_socket, const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);

		void __sendCommandFirst(const boost::system::error_code& error, std::size_t count_send_byte);
		void __reciveCommandFirst(const boost::system::error_code& error, std::size_t count_recive_byte);
		void __sendCommandSecond(const boost::system::error_code& error, std::size_t count_send_byte);
		void __reciveCommandSecond(const boost::system::error_code& error, std::size_t count_recive_byte);
	public:
		Session(boost::asio::ip::tcp::socket& socket, boost::asio::deadline_timer dead_timer, shared_ptr<shared_ptr<map<string, vector<string>>>> lb_worker_data);
		virtual ~Session();
		void start();
		void stop();

		bool isLive();
	};

	class Server : public std::enable_shared_from_this<Server>
	{
	private:
		const int __TIME_KILL = 30;

		shared_ptr <boost::asio::ip::tcp::acceptor> __acceptor;
		shared_ptr <boost::asio::io_context> __context;
		shared_ptr<vector<shared_ptr<Session>>> __sessions;
		shared_ptr<boost::asio::deadline_timer> __kill_timer;

		shared_ptr<shared_ptr<map<string, vector<string>>>> __lb_worker_data;
		void __accept();
		void __killSession(boost::system::error_code error);
	public:
		Server(shared_ptr < boost::asio::io_context> io_context, unsigned short port, shared_ptr<shared_ptr<map<string, vector<string>>>> lb_worker_data);
		virtual ~Server();
		void start();
		void stop();
	};
}