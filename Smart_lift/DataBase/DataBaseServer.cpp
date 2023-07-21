#include "DataBaseServer.hpp"

DataBase::DataBase(string ip, string port, string idBase, net::io_context& ioc)
{
	__endPoint = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ip), stoi(port)));
	__socket = make_shared<tcp::socket>(tcp::socket(ioc));
	__bufRecieve = new char[BUF_SIZE + 1];
	__id = idBase;
	__bufSend = "";
	__bufJsonRecieve = {};
	__parser.reset();
}

DataBase::~DataBase()
{
	this->stop();
	delete[] __bufRecieve;
}

void DataBase::start()
{
	__socket->async_connect(*__endPoint, boost::bind(&DataBase::__reqAutentification, shared_from_this(), boost::placeholders::_1));
}

void DataBase::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
}

void DataBase::__reqAutentification(const boost::system::error_code& eC)
{
	if (eC)
	{
		cerr << eC.message() << endl;
		Sleep(2000);
		this->start();
		return;
	}
	__bufSend = boost::json::serialize()
}