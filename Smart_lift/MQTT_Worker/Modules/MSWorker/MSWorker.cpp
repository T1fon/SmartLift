#include "MSWorker.hpp"
using namespace std;
MSWorker::MSWorker(string ip, string port, string id_worker, boost::asio::io_context& ioc) {
	__end_point = make_shared<net::tcp::endpoint>(net::tcp::endpoint(net::address::from_string(ip), stoi(port)));
	__socket = make_shared<net::tcp::socket>(net::tcp::socket(ioc));
	__buf_recive = new char[BUF_RECIVE_SIZE + 1];
	__id = id_worker;
	__buf_send = "";
	fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
	__buf_json_recive = {};
	__parser.reset();
}
MSWorker::~MSWorker() {
	this->stop();
	delete[] __buf_recive;
}
void MSWorker::start() {
	__socket->async_connect(*__end_point, boost::bind(&MSWorker::__requestAuthentication, shared_from_this(), _1));
}
void MSWorker::stop() {
	if (__socket->is_open()) {
		__socket->close();
	}
}
void MSWorker::__requestAuthentication(const boost::system::error_code& error){
	if (error) {
		cerr << error.what() << endl;
		//чтобы не спамить временное решение
		Sleep(2000);
		this->stop();
		this->start();
		return;
	}
	__buf_send = serialize(json_formatter::worker::request::connect(__WORKER_NAME, __id));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
						 boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1,_2));
}

	//void* a = (void*)(&vector<string>());
	//vector<string>* b = (vector<string>*)a;
	//b->push_back("nu kak tak to 4 kurs ushe");

void MSWorker::__connectAnalize() {
	
	try {
		if (__buf_json_recive.at("response").at("status") == "success") {
			cout << "Connect Successfull!!!!!!" << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
				boost::bind(&MSWorker::__reciveCommand, shared_from_this(), _1, _2));
		}
		else {
			cout << "Connect Fail!!!!!!" << endl;
			cout << __buf_json_recive.at("response").at("message").as_string() << endl;
			//Завершаем работу воркера
			this->stop();
		}
	}
	catch (std::exception &e) {
		cout << "Connect analize " << e.what() << endl;

		this->stop();
		//__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
	//			boost::bind(&MSWorker::__connectAnalize, shared_from_this(), _1, _2));
		return;
	}

}

MSWorker::__CHECK_STATUS MSWorker::__reciveCheck(const size_t& count_recive_byte, __handler_t&& handler) {
	try {
		__parser.write(__buf_recive, count_recive_byte);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
	}
	cout << __buf_recive << endl;
	fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);

	if (!__parser.done()) {
		cerr << "connectAnalize json not full" << endl;
		__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE), handler);
		return __CHECK_STATUS::FAIL;
	}
	try {
		__parser.finish();
		__buf_json_recive = __parser.release();
		__parser.reset();
	}
	catch (exception& e) {
		__parser.reset();
		__buf_json_recive = {};
		cerr << "_reciveCheck " << e.what();
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}
MSWorker::__CHECK_STATUS MSWorker::__sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler) {
	temp_send_byte += count_send_byte;
	if (__buf_send.size() != temp_send_byte) {
		__socket->async_send(boost::asio::buffer(__buf_send.c_str() + temp_send_byte, (__buf_send.size() - temp_send_byte)), handler);
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}

void MSWorker::__sendCommand(const boost::system::error_code& error, std::size_t count_send_byte) {
	if (error) {
		cerr << "sendCommand " << error.what() << endl;
		this->stop();
		this->start();
		return;
	}

	static size_t temp_send_byte = 0;
	if(__sendCheck(count_send_byte,temp_send_byte, boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2)) == __CHECK_STATUS::FAIL){
		return;
	}
	temp_send_byte = 0;
	__buf_send = "";
	__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
		boost::bind(&MSWorker::__reciveCommand, shared_from_this(), _1, _2));
}

void MSWorker::__reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte) {
	if (error) {
		cerr << "reciveCommand " << error.what() << endl;
		this->stop();
		this->start();
		return;
	}
	
	if(__reciveCheck(count_recive_byte, boost::bind(&MSWorker::__reciveCommand, shared_from_this(), _1, _2)) == __CHECK_STATUS::FAIL)
	{return;}

	__commandAnalize();
	__buf_json_recive = {};	
}

void MSWorker::__commandAnalize() {
	try {
		boost::json::value target = __buf_json_recive.at("target");
		
		if (target == "ping") {
			__responsePing();
		}
		else if(target == "Move_lift") {
			
		}
		else if (target == "disconnect") {
			__responseDisconnect();
		}
		else if (target == "connect") {
			__connectAnalize();
		}
	}
	catch (std::exception &e) {
		cout << "Command analize " << e.what() << endl;
		return;
	}
}
void MSWorker::__responsePing() {
	__buf_send = serialize(json_formatter::worker::response::ping(__WORKER_NAME));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
		boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2));
}
void MSWorker::__responseDisconnect() {
	__buf_send = serialize(json_formatter::worker::response::disconnect(__WORKER_NAME));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
		boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2));
}
