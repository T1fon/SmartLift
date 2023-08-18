#include "NetRepeater.hpp"

namespace net_repeater {
	Server::Server(shared_ptr < boost::asio::io_context> io_context, unsigned short port, std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt_worker)
	{
		__context = io_context;
		__acceptor = make_shared<boost::asio::ip::tcp::acceptor>(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
		__kill_timer = make_shared<boost::asio::deadline_timer>(*__context);
		__sessions = make_shared<std::vector<std::shared_ptr<Session>>>();
		__sessions_mqtt_worker = sessions_mqtt_worker;
	}
	Server::~Server(){
		this->stop();
	}
	void Server::start( shared_ptr<shared_ptr<map<string, vector<string>>>> db_worker_lu,
						shared_ptr<shared_ptr<map<string, vector<string>>>> db_lift_blocks)
	{
		__db_worker_lu = db_worker_lu;
		__db_lift_blocks = db_lift_blocks;
		__accept();
		__kill_timer->expires_from_now(boost::posix_time::seconds(__TIME_KILL));
		__kill_timer->async_wait(boost::bind(&Server::__killSession, shared_from_this(), _1));
	}
	void Server::stop(){
		for (auto i = __sessions->begin(), end = __sessions->end(); i != end; i++) {
			(*i)->stop();
		}
		__sessions.reset();
	}
	void Server::__accept() {
		__acceptor->async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
			{
				if (error) {
					cerr << error.message() << endl;
					__accept();
				}
				__sessions->push_back(std::make_shared<Session>(socket,boost::asio::ip::tcp::socket(*__context), boost::asio::deadline_timer(*__context)));
				__sessions->back()->start(__db_worker_lu, __db_lift_blocks);
				__accept();
			});
	}
	void Server::__killSession(boost::system::error_code error) {
		bool kill = false;
		for (auto i = __sessions->begin(), end = __sessions->end(); i != end; i++) {
			if (!(*i)->isLive()) {
				kill = true;
				break;
			}
		}

		if (kill) {
			__kill_timer->expires_from_now(boost::posix_time::seconds(1));
			__kill_timer->async_wait(boost::bind(&Server::__killSession, shared_from_this(), _1));
		}
		else {
			__kill_timer->expires_from_now(boost::posix_time::seconds(__TIME_KILL));
			__kill_timer->async_wait(boost::bind(&Server::__killSession, shared_from_this(), _1));
		}
	}

	/*-------------------------------------------------------*/

	Session::Session(boost::asio::ip::tcp::socket& socket, boost::asio::ip::tcp::socket& socket_second, boost::asio::deadline_timer dead_timer, std::shared_ptr<std::vector<std::shared_ptr<worker_server::Session>>> sessions_mqtt_worker) :
		__dead_timer(std::move(dead_timer))
	{
		__socket_first = make_shared<boost::asio::ip::tcp::socket>(move(socket));
		__socket_second = make_shared<boost::asio::ip::tcp::socket>(move(socket_second));
		__sessions_mqtt_worker = sessions_mqtt_worker;
		__is_live = false;
		__buf_recive_first = new char[__BUF_RECIVE_SIZE + 1];
		__buf_recive_second = new char[__BUF_RECIVE_SIZE + 1];
		fill_n(__buf_recive_first, __BUF_RECIVE_SIZE, 0);
		fill_n(__buf_recive_second, __BUF_RECIVE_SIZE, 0);
	}
	Session::~Session() { 
		stop();
		delete[] __buf_recive_first;
		delete[] __buf_recive_second;
	}
	void Session::start(shared_ptr<shared_ptr<map<string, vector<string>>>> db_worker_lu,
						shared_ptr<shared_ptr<map<string, vector<string>>>> db_lift_blocks)
	{
		__db_worker_lu = db_worker_lu;
		__db_lift_blocks = db_lift_blocks;
		__is_live = true;
		__its_connect = true;
		__socket_first->async_receive(boost::asio::buffer(__buf_recive_first, __BUF_RECIVE_SIZE), boost::bind(&Session::__reciveCommandFirst, shared_from_this(), _1, _2));
	}
	void Session::stop(){
		if (__socket_first->is_open()) {
			__socket_first->close();
		}
		if (__socket_second) {
			if (__socket_second->is_open()) {
				__socket_second->close();
			}
		}
		__is_live = false;
	}

	bool Session::isLive(){
		return __is_live;
	}

	
	Session::__CHECK_STATUS Session::__sendCheck(__SOCKET num_socket, const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler)
	{
		temp_send_byte += count_send_byte;
		switch (num_socket) {
		case __SOCKET::FIRST:
			if (__buf_send_first.size() != temp_send_byte) {
				__socket_first->async_send(boost::asio::buffer(__buf_send_first.c_str() + temp_send_byte, (__buf_send_first.size() - temp_send_byte)), handler);
				return __CHECK_STATUS::FAIL;
			}
			break;
		case __SOCKET::SECOND:
			if (__buf_send_second.size() != temp_send_byte) {
				__socket_second->async_send(boost::asio::buffer(__buf_send_second.c_str() + temp_send_byte, (__buf_send_second.size() - temp_send_byte)), handler);
				return __CHECK_STATUS::FAIL;
			}
			break;
		}
		
		return __CHECK_STATUS::SUCCESS;
	}

	void Session::__sendCommandFirst(const boost::system::error_code& error, std::size_t count_send_byte){
		if (error) {
			cerr << "sendCommand " << error.what() << endl;
			this->stop();
			return;
		}

		static size_t temp_send_byte = 0;
		if (__sendCheck(__SOCKET::FIRST,count_send_byte, temp_send_byte, boost::bind(&Session::__sendCommandFirst, shared_from_this(), _1, _2)) == __CHECK_STATUS::FAIL) {
			return;
		}
		temp_send_byte = 0;
		__buf_send_first = "";
		fill_n(__buf_recive_first, __BUF_RECIVE_SIZE, 0);
		//__socket.async_receive(boost::asio::buffer(__buf_recive, __BUF_RECIVE_SIZE),
		///	boost::bind(&Session::__reciveCommand, shared_from_this(), _1, _2));
		
	}
	void Session::__reciveCommandFirst(const boost::system::error_code& error, std::size_t count_recive_byte){
		if (error) {
			cerr << "_reciveCommand " << error.what() << endl;
			this->stop();
			return;
		}

		if (__its_connect) {
			/*проверяем можем ли мы обработать этот лифтовой блок*/

			const short MQTT_SHIFT = 14;
			const short SEPARATING_CHAR = 32;
			string authorization_data(__buf_recive_first + MQTT_SHIFT);
			authorization_data = authorization_data.substr(0, authorization_data.find(SEPARATING_CHAR));
			string worker_id;
			short count_found = 0;
			try {
				if (authorization_data.substr(0, 7) == "LKDS_LU") {
					string descriptor = "\"" + authorization_data.substr(7) + "\"";
					bool found = false;
					string temp_id;
					for (auto i = (*__db_lift_blocks)->at("Descriptor").begin(),
						j = (*__db_lift_blocks)->at("WorkerLuId").begin(),
						k = (*__db_lift_blocks)->at("WorkerLuSecId").begin(), end = (*__db_lift_blocks)->at("Descriptor").end(); i != end; i++, j++, k++)
					{
						if (descriptor == *i) {
							for (auto m = __sessions_mqtt_worker->begin(), end_m = __sessions_mqtt_worker->end(); m != end_m || count_found < 2; m++) {
								temp_id = (*m)->getId();
								if (temp_id == *j) {
									worker_id = *j;
									found = true;
									break;
								}
								else if (temp_id == *k) {
									worker_id = *k;
									found = true;
									break;
								}
							}
							break;
						}
					}
					if (!found) {
						this->stop();
						return;
					}
				}
				else {
					this->stop();
					return;
				}
			}
			catch (exception& e) {
				cout << "NET REPEATER __reciveCommandFirst (__db_lift_blocks): " << e.what() << endl;
			}
			
			string worker_ip;
			string worker_port;
			try {
				for (auto i = (*__db_worker_lu)->at("WorkerLuId").begin(),
					j = (*__db_worker_lu)->at("WorkerLuAdd").begin(),
					k = (*__db_worker_lu)->at("WorkerLuPort").begin(),
					end = (*__db_worker_lu)->at("WorkerLuId").end(); i != end; i++, j++, k++)
				{
					if (*i == worker_id) {
						worker_ip = *j;
						worker_port = *k;
						break;
					}
				}
			}
			catch (exception& e) {
				cout << "NET REPEATER __reciveCommandFirst (__db_worker_lu): " << e.what() << endl;
			}

			__its_connect = false;
			__buf_send_second = __buf_recive_first;
			fill_n(__buf_recive_first, __BUF_RECIVE_SIZE, 0);
			__socket_second->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(worker_ip), stoi(worker_port)),);

			

			/*----------------------------*/
			
			//__socket.async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
			//	boost::bind(&Session::__reciveCommand, shared_from_this(), _1, _2));
			//__socket.async_receive(boost::asio::buffer(__buf_recive,__BUF_RECIVE_SIZE),
			//	boost::bind(&Session::__reciveCommand, shared_from_this(), _1, _2));
			
			
		}
		//__buf_send = __buf_recive;
		//fill_n(__buf_recive, __BUF_RECIVE_SIZE, 0);
		//__socket.async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
		//	boost::bind(&Session::__reciveCommand, shared_from_this(), _1, _2));
	}
}
