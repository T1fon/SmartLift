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
		__socket->async_send(boost::asio::buffer(__bufQueueString, __bufQueueString.size()),
			boost::bind(&Client::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	else
	{
		cerr << "NO SEND DATA";
		this->stop();
		this->start();
		return;
	}
}

Client::__CHECK_STATUS Client::__checkSend(const size_t& countSendByte, size_t& tempSendByte, __handler_t&& handler)
{
	tempSendByte += countSendByte;
	if (__bufQueueString.size() != tempSendByte)
	{
		__socket->async_send(net::buffer(__bufQueueString.c_str() + tempSendByte, (__bufQueueString.size() - tempSendByte)), handler);
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}

Client::__CHECK_STATUS Client::__checkJson(const size_t& countReciveByte, __handler_t&& handler)
{
	try
	{
		__parser.write(__bufRecive, countReciveByte);
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
	fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
	if (!__parser.done())
	{
		cerr << "checkConnect, Json is not full" << endl;
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), handler);
		return __CHECK_STATUS::FAIL;
	}
	try
	{
		__parser.finish();
		__bufJsonRecive = __parser.release();
		__parser.reset();
	}
	catch (exception& e)
	{
		__parser.reset();
		__bufJsonRecive = {};
		cerr << "checkConnect" << e.what() << endl;
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
	if (__checkSend(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL)
	{
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
	if (__checkJson(bytesRecive, boost::bind(&Client::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)) == __CHECK_STATUS::FAIL)
	{
		return;
	}
	__commandAnalize();
	__bufJsonRecive = {};
}

void Client::__commandAnalize()
{
	boost::json::value target = __bufJsonRecive.at("target");
	boost::json::value status = __bufJsonRecive.at("response").at("status");
	if (target == "connect")
	{
		{
			if (status != "success")
			{
				__socket->async_send(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
				this->stop();
				this->start();
				return;
			}
			else
			{
				__socket->async_send(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
		}
	}
	else if (target == "disconnect")
	{
		if (status == "success")
		{
			__socket->async_send(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			this->stop();
			this->start();
			return;
		}
		else
		{
			__socket->async_send(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Client::__reqConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
	}

}