/*#include "Worker.hpp"

Worker::Worker(shared_ptr<tcp::socket> socket, map<string, string> confInfo, net::io_context& ioc) : __socket(move(socket)), __ioc(ioc)
{
	__bufRecive = new char[BUF_RECIVE_SIZE + 1];
	__bufSend = "";
	__bufJsonRecive = {};
	__parser.reset();
	__configInfo = confInfo;
}

void Worker::start()
{
	//__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	__connectToMS();
}

void Worker::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
}

Worker::~Worker()
{
	this->stop();
	delete[] __bufRecive;
}

Worker::__CHECK_STATUS Worker::__checkJson(const size_t& countReciveByte, __handler_t&& handler)
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

Worker::__CHECK_STATUS Worker::__checkSend(const size_t& countSendByte, size_t& tempSendByte, __handler_t&& handler)
{
	tempSendByte += countSendByte;
	if (__bufSend.size() != tempSendByte)
	{
		__socket->async_send(net::buffer(__bufSend.c_str() + tempSendByte, (__bufSend.size() - tempSendByte)), handler);
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}

void Worker::__connectToMS()
{
	string ipMS = __configInfo.at("Main_server_ip");
	string portMS = __configInfo.at("Main_server_port");
	string workerId = __configInfo.at("Id");
	queue<string> queryMS;
	string bufQueue = boost::json::serialize(json_formatter::worker::request::connect(__name, workerId));
	queryMS.push(bufQueue);
	//make_shared<Client>(ipMS, portMS, __ioc, queryMS)->start();
	__socket->async_send(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2)); //Тут нужен сенд или только прием?
}

void Worker::__recieveConnectToMS(const boost::system::error_code& eC, size_t bytesRecive)
{
	if (eC)
	{
		cerr << "Error_connect_to_MS " << eC.what() << endl;
		Sleep(2000);
		this->stop();
		this->start();
		return;
	}
	if (__checkJson(bytesRecive, boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2)) == __CHECK_STATUS::FAIL)
	{
		return;
	}
	boost::json::value status = __bufJsonRecive.at("response").at("status");
	if (status == "success")
	{
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
			boost::placeholders::_1, boost::placeholders::_2));
	}
	else
	{
		cerr << "No_permission_to_Connect" << endl;
		Sleep(1000);
		__connectToMS();
	}
}

void Worker::__connectToBd()
{
	string ipBd = __configInfo.at("BD_ip");
	string portBd = __configInfo.at("BD_port");
	string BDLog = __configInfo.at("BD_login");
	string BDPas = __configInfo.at("BD_password");
	string Connect = boost::json::serialize(json_formatter::database::request::connect(__name, BDLog, BDPas));
	//string Select = "SELECT * FROM "
	string disconnect = boost::json::serialize(json_formatter::database::request::disconnect(__name));
	queue<string> requestBody;
	requestBody.push(Connect);
	//все селекты
	requestBody.push(disconnect);
	//make_shared<Client>(ipBd, portBd, __ioc, requestBody)->start();
	__socket->async_send(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__recieveConnectToBd, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2)); //Тут нужен сенд или только прием?
}

void Worker::__recieveConnectToBd(const boost::system::error_code& eC, size_t bytesRecive)
{
	if (eC)
	{
		cerr << "Error_connect_to_MS " << eC.what() << endl;
		Sleep(2000);
		this->stop();
		this->__waitCommand(eC, bytesRecive);
		return;
	}
	if (__checkJson(bytesRecive, boost::bind(&Worker::__recieveConnectToBd, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2)) == __CHECK_STATUS::FAIL)
	{
		return;
	}


}

void Worker::__waitCommand(const boost::system::error_code& eC, size_t bytesRecive)
{
	if (eC)
	{
		cerr << "waitCommand " << eC.what() << endl;
		Sleep(1000);
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
			boost::placeholders::_1, boost::placeholders::_2));
		return;
	}

	if (__checkJson(bytesRecive, boost::bind(&Worker::__waitCommand, shared_from_this(), 
		boost::placeholders::_1, boost::placeholders::_2)) == __CHECK_STATUS::FAIL)
	{
		return;
	}
	boost::json::value sender = __bufJsonRecive.at("sender");
	boost::json::value target = __bufJsonRecive.at("target");
	boost::json::value status = __bufJsonRecive.at("response").at("status");
	if (target == "ping")
	{
		if (status == "")
		{
			__makePing();
		}
		else if (status == "success")
		{
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
		}
		else if (status == "fail")
		{
			cerr << "fail ping" << endl;
			__makePing();
		}
	}
	else if (target == "disconnect")
	{
		if (status == "")
		{
			__makeDisconnect();
		}
		else if (status == "success")
		{
			this->stop();
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
		}
		else if (status == "fail")
		{
			boost::json::value fail = __bufJsonRecive.at("response").at("message");
			cerr << "fail disconnect " << fail << endl;
			__makeDisconnect();

		}
	}
	else if (target == "marussia_station_request")
	{
		__makeRequest();
	}
	else if (target == "db_query")
	{
		__readQuery();
	}
	else if (target == "connect")
	{
		if (status == "")
		{
			if (sender == "Main_server")
			{
				__connectToMS();
			}
		}
		else if (status == "success")
		{
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
		}
		else if (status == "fail")
		{
			if (sender == "Main_server")
			{
				boost::json::value fail = __bufJsonRecive.at("response").at("message");
				cerr << "error connect " << fail  << endl;
				__connectToMS();
			}
			else if (sender == "Data_base")
			{
				boost::json::value fail = __bufJsonRecive.at("response").at("message");
				cerr << "error connect " << fail << endl;
				__connectToBd();
			}
		}
	}

	//делаем таймер на работу бд
	__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2));
}

void Worker::__makePing()
{

}*/