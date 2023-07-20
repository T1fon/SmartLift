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
void MSWorker::__requestAuthentication(const boost::system::error_code& error){
	if (error) {
		cerr << error.message() << endl;
		//чтобы не спамить временное решение
		Sleep(2000);
		this->start();
		return;
	}
	__buf_send = serialize(json_formatter::worker::request::connect(__WORKER_NAME, __id));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
						 boost::bind(&MSWorker::__responseAuthentication, shared_from_this(), _1,_2));
}
void MSWorker::__responseAuthentication(const boost::system::error_code& error, std::size_t count_send_byte) {
	if (error) {
		cerr << error.message() << endl;
		//чтобы не спамить временное решение
		Sleep(2000);
		__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
			boost::bind(&MSWorker::__responseAuthentication, shared_from_this(), _1, _2));
		return;
	}


	static size_t temp_send_byte = 0;
	temp_send_byte += count_send_byte;
	if (__buf_send.size() != temp_send_byte) {
		__socket->async_send(boost::asio::buffer(__buf_send.c_str()+temp_send_byte, (__buf_send.size() - temp_send_byte)),
			boost::bind(&MSWorker::__responseAuthentication, shared_from_this(), _1, _2));
		return;
	}
	temp_send_byte = 0;
	__buf_send = "";
	__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
							boost::bind(&MSWorker::__connectAnalize, shared_from_this(), _1, _2));
}
void MSWorker::__connectAnalize(const boost::system::error_code& error, std::size_t count_recive_byte) {
	if (error) {
		cerr << error.message() << endl;
		Sleep(1000);
		__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
			boost::bind(&MSWorker::__connectAnalize, shared_from_this(), _1, _2));
		return;
	}
	
	try {
		__parser.write(__buf_recive,count_recive_byte);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
	}
	
	fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
	if (!__parser.done()) {
		cerr << "connectAnalize json not full" << endl;
		__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
			boost::bind(&MSWorker::__connectAnalize, shared_from_this(), _1,_2));
		return;
	}

	try {
		__parser.finish();
		__buf_json_recive = __parser.release();
		__parser.reset();
		
		if (__buf_json_recive.at("response").at("status").as_string() == "success") {
			cout << "Connect Successfull!!!!!!" << endl;
		}
		else {
			cout << "Connect Fail!!!!!!" << endl;
			cout << __buf_json_recive.at("response").at("message").as_string() << endl;
			//Завершаем работу воркера
			this->stop();
		}
	}
	catch (std::exception e) {
		cout << e.what() << endl;

		__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
			boost::bind(&MSWorker::__connectAnalize, shared_from_this(), _1, _2));
	}

	__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
							boost::bind(&MSWorker::__waitCommand, shared_from_this(), _1, _2));
}

void MSWorker::__waitCommand(const boost::system::error_code& error, std::size_t count_recive_byte) {
	if (error) {
		cerr << error.message() << endl;
		Sleep(1000);
		__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
			boost::bind(&MSWorker::__waitCommand, shared_from_this(), _1, _2));
		return;
	}
	
	
	//ожидаем новой команды
	__socket->async_receive(boost::asio::buffer(__buf_recive, BUF_RECIVE_SIZE),
		boost::bind(&MSWorker::__waitCommand, shared_from_this(), _1, _2));
}

void MSWorker::start() {
	__socket->async_connect(*__end_point, boost::bind(&MSWorker::__requestAuthentication, shared_from_this(), _1));
}
void MSWorker::stop() {
	if(__socket->is_open()) {
		__socket->close();
	}
}