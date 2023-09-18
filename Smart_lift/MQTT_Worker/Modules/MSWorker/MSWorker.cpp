#include "MSWorker.hpp"
using namespace std;
MSWorker::MSWorker(string ip, string port, string id_worker, boost::asio::io_context& ioc)
{
	__end_point = make_shared<net_ip::tcp::endpoint>(net_ip::tcp::endpoint(net_ip::address::from_string(ip), stoi(port)));
	__socket = make_shared<net_ip::tcp::socket>(net_ip::tcp::socket(ioc));
	__buf_recive = new char[__BUF_RECIVE_SIZE + 1];
	__id = id_worker;
	__buf_send = "";
	fill_n(__buf_recive, __BUF_RECIVE_SIZE, 0);
	__buf_json_recive = {};
	__json_parser.reset();
	__callback_mqtt_worker = bind(&MSWorker::__emptyCallback, this, _1, _2, _3);
}
MSWorker::~MSWorker() {
	this->stop();
	delete[] __buf_recive;
}
void MSWorker::start(std::shared_ptr<std::shared_ptr<std::map<std::string, std::string>>> lu_id_descriptor) {
	cout << "Start" << endl;
	__lu_id_descriptor = lu_id_descriptor;
	__socket->async_connect(*__end_point, boost::bind(&MSWorker::__requestAuthentication, shared_from_this(), _1));
}
void MSWorker::stop() {
	try{
        //__socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        //__socket->shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
        if (__socket->is_open()) {
            __socket->close();
        }
    }
    catch(exception &e){
        cerr << "stop operation Worker server" << endl;
    }
}
void MSWorker::__requestAuthentication(const boost::system::error_code& error){
	if (error) {
		cerr << error.what() << endl;
		//����� �� ������� ��������� �������
		
		#ifndef UNIX
			sleep(2);
		#else
			Sleep(2000);
		#endif
		this->stop();
		this->start(__lu_id_descriptor);
		return;
	}
	__buf_send = serialize(json_formatter::worker::request::connect(__WORKER_NAME, __id));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
						 boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1,_2));
}

void MSWorker::__connectAnalize() {
	
	try {
		if (__buf_json_recive.front().at("response").at("status") == "success") {
			
			cout << "Connect Successfull!!!!!!" << endl;
			__socket->async_receive(boost::asio::buffer(__buf_recive, __BUF_RECIVE_SIZE),
				boost::bind(&MSWorker::__reciveCommand, shared_from_this(), _1, _2));
		}
		else {
			cout << "Connect Fail!!!!!!" << endl;
			cout << __buf_json_recive.front().at("response").at("message").as_string() << endl;
			//��������� ������ �������
			this->stop();
		}
		__buf_json_recive.pop();
	}
	catch (std::exception &e) {
		cout << "Connect analize " << e.what() << endl;
		__buf_json_recive.pop();
		this->stop();
		return;
	}

}
void MSWorker::__emptyCallback(std::string lu_id, std::string floor_number, std::string station_id) {
	cout << "called __emptyCallback " << lu_id << " "<<floor_number << " " << station_id <<  endl;
}

MSWorker::__CHECK_STATUS MSWorker::__reciveCheck(const size_t& count_recive_byte, __handler_t&& handler) {
	size_t count_byte, count_byte_write = 0;
	for(; count_byte_write < count_recive_byte;){
		try {
            count_byte = __json_parser.write_some(__buf_recive + count_byte_write);
        }
        catch (exception& e) {
            cerr << e.what() << endl;
        }

		if (!__json_parser.done()) {
            fill_n(__buf_recive, __BUF_RECIVE_SIZE, 0);
            cerr << "_reciveCheck json not full " << endl;
            __socket->async_receive(boost::asio::buffer(__buf_recive, __BUF_RECIVE_SIZE), handler);
            return __CHECK_STATUS::FAIL;
        }
        try {
            __buf_json_recive.push(__json_parser.release());
            __json_parser.reset();
            count_byte_write += count_byte;
        }
        catch (exception& e) {
            __json_parser.reset();
            __buf_json_recive = {};
            cerr << "_reciveCheck " << e.what();
            return __CHECK_STATUS::FAIL;
        }	
	}

	fill_n(__buf_recive, __BUF_RECIVE_SIZE, 0);
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
		cerr << "__sendCommand " << error.what() << endl;
		
		this->stop();
		this->start(__lu_id_descriptor);
		return;
	}

	static size_t temp_send_byte = 0;
	if(__sendCheck(count_send_byte,temp_send_byte, boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2)) == __CHECK_STATUS::FAIL){
		return;
	}
	temp_send_byte = 0;
	__buf_send = "";
	__socket->async_receive(boost::asio::buffer(__buf_recive, __BUF_RECIVE_SIZE),
		boost::bind(&MSWorker::__reciveCommand, shared_from_this(), _1, _2));
}

void MSWorker::__reciveCommand(const boost::system::error_code& error, std::size_t count_recive_byte) {
	if (error) {
		cerr << "reciveCommand " << error.what() << endl;
		
		this->stop();
		this->start(__lu_id_descriptor);
		return;
	}
	
	if(__reciveCheck(count_recive_byte, boost::bind(&MSWorker::__reciveCommand, shared_from_this(), _1, _2)) == __CHECK_STATUS::FAIL)
	{return;}

	__commandAnalize();
	__buf_json_recive = {};	
}

void MSWorker::__commandAnalize() {
	for (; __buf_json_recive.size() > 0;) {
		try {
			boost::json::value target = __buf_json_recive.front().at("target");
			if (target == "ping") {
				__responsePing();
			}
			else if(target == "move_lift") {
				__moveLift();
			}
			else if (target == "disconnect") {
				__responseDisconnect();
			}
			else if (target == "connect") {
				__connectAnalize();
			}
			else {
				cout << "JA NE ZNAU TAKOI CELI" << endl;
			}
		}
		catch (std::exception &e) {
			cout << "Command analize " << e.what() << endl;
			return;
		}
	}
	
}
void MSWorker::__responsePing() {
	__buf_json_recive.pop();
	__buf_send = serialize(json_formatter::worker::response::ping(__WORKER_NAME));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
		boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2));
}
void MSWorker::__responseDisconnect() {
	__buf_json_recive.pop();
	__buf_send = serialize(json_formatter::worker::response::disconnect(__WORKER_NAME));
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
		boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2));
}
void MSWorker::__moveLift() {
	bool found = false;
	std::string lb_descriptor = "";
	try {
		lb_descriptor = (*__lu_id_descriptor)->at(__buf_json_recive.front().at("request").at("mqtt_command").at("lb_id").as_string().c_str());
		found = true;
	}
	catch (exception& e) {
		cerr << "__moveLift: " << e.what() << endl;
	};
	if (!found) {
		__buf_send = serialize(json_formatter::worker::response::mqtt_lift_move(__WORKER_NAME, __buf_json_recive.front().at("request").at("station_id").as_string().c_str(),
																				json_formatter::STATUS_OPERATION::fail, "LU not found"));
		__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
			boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2));
		return;
	}
	
	__callback_mqtt_worker( lb_descriptor,
						    __buf_json_recive.front().at("request").at("mqtt_command").at("value").as_string().c_str(),
							__buf_json_recive.front().at("request").at("station_id").as_string().c_str());
	__buf_json_recive.pop();
}
void MSWorker::setCallback(callback_mqtt_worker_t callback_mqtt_worker) {
	__callback_mqtt_worker = callback_mqtt_worker;
}
void MSWorker::responseMove(std::string station_id, bool success) {
	if(success){
		__buf_send = serialize(json_formatter::worker::response::mqtt_lift_move(__WORKER_NAME, station_id,
		json_formatter::STATUS_OPERATION::success));
	}
	else{
		__buf_send = serialize(json_formatter::worker::response::mqtt_lift_move(__WORKER_NAME, station_id,
		json_formatter::STATUS_OPERATION::fail));
	}
	
	__socket->async_send(boost::asio::buffer(__buf_send, __buf_send.size()),
		boost::bind(&MSWorker::__sendCommand, shared_from_this(), _1, _2));
}
