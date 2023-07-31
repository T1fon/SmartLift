#include "newClient.hpp"

Client::Client(string ip, string port, net::io_context& ioc, queue<string> message)
{
	__endPoint = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ip), stoi(port)));
	__socket = make_shared<tcp::socket>(tcp::socket(ioc));
	__bufRecive = new char[BUF_RECIVE_SIZE + 1];
	__bufSend = message;
	fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
	__bufJsonRecive = {};
	__parser.reset();
}

void Client::start()
{
	__socket->async_connect(*__endPoint, boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1));
}

void Client::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
}

Client::~Client()
{
	this->stop();
	delete[] __bufRecive;
}

void Client::__reqConnect(const boost::system::error_code& eC)
{
	if (eC) 
	{
		cerr << eC.what() << endl;
		Sleep(2000);
		this->stop();
		this->start();
		return;
	}
	if(!__bufSend.empty())
	{
		__bufQueueString = __bufSend.front();
		__bufSend.pop();
		__socket->async_send(boost::asio::buffer(__bufQueueString, __bufQueueString.size()),boost::bind(&Client::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	else
	{
		cerr << "NO SEND DATA";
		this->stop();

		return;
	}
}

Client::__CHECK_STATUS Client::__sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler) 
{
	temp_send_byte += count_send_byte;
	if (__bufQueueString.size() != temp_send_byte) {
		__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + temp_send_byte, (__bufQueueString.size() - temp_send_byte)), handler);
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}

Client::__CHECK_STATUS Client::__reciveCheck(const size_t& count_recive_byte, __handler_t&& handler) 
{
	try {
		__parser.write(__bufRecive, count_recive_byte);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
	}
	cout << __bufRecive << endl;
	fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);

	if (!__parser.done()) {
		cerr << "connectAnalize json not full" << endl;
		__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), handler);
		return __CHECK_STATUS::FAIL;
	}
	try {
		__parser.finish();
		__bufJsonRecive = __parser.release();
		__parser.reset();
	}
	catch (exception& e) {
		__parser.reset();
		__bufJsonRecive = {};
		cerr << "_reciveCheck " << e.what();
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}

void Client::__sendConnect(const boost::system::error_code& eC, size_t bytesSend)
{
	if (eC)
	{
		cerr << "sendConnect" << eC.what() << endl;
		this->stop();
		this->start();
		return;
	}
	static size_t tempBytesSend = 0;
	/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
		return;
	}*/
	tempBytesSend += bytesSend;
	if (__bufQueueString.size() != tempBytesSend) {
		__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + tempBytesSend, (__bufQueueString.size() - tempBytesSend)),
			boost::bind(&Client::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	tempBytesSend = 0;
	__bufQueueString.clear();
	__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void Client::__reciveCommand(const boost::system::error_code& eC, size_t bytesRecive)
{
	if (eC)
	{
		cerr << "reciveCommand " << eC.what() << endl;
		this->stop();
		this->start();
		return;
	}
	/*if (__reciveCheck(bytesRecive, boost::bind(&Client::__reciveCommand, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL)
	{
		return;
	}*/
	try {
		__parser.write(__bufRecive, bytesRecive);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
	}
	cout << __bufRecive << endl;
	fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);

	if (!__parser.done()) {
		cerr << "connectAnalize json not full" << endl;
		__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reciveCommand, shared_from_this(), 
			boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	try {
		__parser.finish();
		__bufJsonRecive = __parser.release();
		__parser.reset();
	}
	catch (exception& e) {
		__parser.reset();
		__bufJsonRecive = {};
		cerr << "_reciveCheck " << e.what();
		return;
	}
	cout << __bufJsonRecive << endl;
	__commandAnalize();
}

void Client::__commandAnalize()
{
	boost::json::value target = __bufJsonRecive.at("target");
	boost::json::value status = __bufJsonRecive.at("response").at("status");
	__bufQueueString.clear();
	__bufQueueString = __bufJsonRecive.as_string();
	if (target == "connect")
	{
		{
			if (status != "success")
			{
				__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1));
				this->stop();
				return;
			}
			else
			{
				__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1));
			}
		}
	}
	else if (target == "disconnect")
	{
		if (status == "success")
		{
			__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1));
			this->stop();
			return;
		}
		else
		{
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1));
		}
	}

}