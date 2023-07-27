#include "DataBaseServer.hpp"

DataBase::DataBase(shared_ptr<Log> lg, shared_ptr<tcp::socket> socket): __socket(move(socket))
{
	__log = lg;
	__bufRecieve = new char[BUF_SIZE + 1];

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
	__log->writeLog(0, "DataBase", "Connect_with_DataBase");
	__log->writeTempLog(0, "DataBase", "Connect_with_DataBase");
	__socket->async_connect(*__endPoint, boost::bind(&DataBase::__reqAutentification, shared_from_this(), boost::placeholders::_1));
}

void DataBase::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
	__log->writeLog(0, "DataBase", "End_Connect");
	__log->writeTempLog(0, "DataBase", "End_Connect");
}

void DataBase::__reqAutentification(const boost::system::error_code& eC)
{
	if (eC)
	{
		cerr << eC.message() << endl;
		Sleep(2000);
		this->start();
		__log->writeLog(3, "DataBase", "Error_connect");
		__log->writeTempLog(3, "DataBase", "Error_connect");
		return;
	}
	__bufSend = "";
	__socket->async_receive(net::buffer(__bufRecieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void DataBase::__connectAnalize(const boost::system::error_code& eC, size_t bytesRecieve)
{
	if (eC)
	{
		cerr << eC.message() << endl;
		Sleep(1000);
		__log->writeLog(2, "DataBase", "Error_failed_to_read_request");
		__log->writeTempLog(2, "DataBase", "Error_failed_to_read_request");
		__socket->async_receive(net::buffer(__bufRecieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	try
	{
		__parser.write(__bufRecieve, bytesRecieve);
	}
	catch(exception& e)
	{
		cerr << e.what() << endl;
	}
	fill_n(__bufRecieve, BUF_SIZE, 0);
	if (!__parser.done())
	{
		cerr << "Connect analize json not full" << endl;
		__log->writeTempLog(2, "DataBase", "Not_full_json");
		__socket->async_receive(net::buffer(__bufRecieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	try
	{
		__parser.finish();
		__bufJsonRecieve = __parser.release();
		__parser.reset();

		string login = boost::json::serialize(__bufJsonRecieve.at("request").at("login"));
		string password = boost::json::serialize(__bufJsonRecieve.at("request").at("password"));
		__checkConnect(login, password);
		__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&DataBase::__resAutentification, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		if (__flagWrongConnect)
		{
			stop();
			__flagWrongConnect = false;
		}
		__log->writeTempLog(0, "DataBase", "send_answer_to_connect");
		
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
		__socket->async_receive(net::buffer(__bufRecieve, BUF_SIZE), boost::bind(&DataBase::__connectAnalize, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
}

void DataBase::__resAutentification(const boost::system::error_code& eC, size_t bytesSend)
{
	if (eC)
	{
		__log->writeLog(2, "DataBase", "Error_failed_to_read_response");
		__log->writeTempLog(2, "DataBase", "Error_failed_to_read_response");
		cerr << eC.message() << endl;
		Sleep(2000);
		__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&DataBase::__resAutentification, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	static size_t tempSend = 0;
	tempSend += bytesSend;
	if (tempSend != __bufSend.size())
	{
		__log->writeTempLog(0, "DataBase", "Not_full_json");
		__socket->async_send(net::buffer(__bufSend.c_str() + tempSend, __bufSend.size() - tempSend), boost::bind(&DataBase::__resAutentification, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	tempSend = 0;
	__bufSend.clear();
	__log->writeTempLog(2, "DataBase", "Receiving_a_request");
	__socket->async_receive(net::buffer(__bufRecieve, BUF_SIZE), boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void DataBase::__waitCommand(const boost::system::error_code& eC, size_t bytesRecieve)
{
	if (eC) {
		__log->writeLog(2, "DataBase", "Error_failed_to_read_request");
		__log->writeTempLog(2, "DataBase", "Error_failed_to_read_request");
		cerr << eC.message() << endl;
		Sleep(1000);
		__socket->async_receive(boost::asio::buffer(__bufRecieve, BUF_SIZE),
			boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}

	try
	{
		__parser.write(__bufRecieve, bytesRecieve);
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
	fill_n(__bufRecieve, BUF_SIZE, 0);
	if (!__parser.done())
	{
		cerr << "waitCommand json is not full" << endl;
		__log->writeTempLog(0, "DataBase", "Not_full_json");
		__socket->async_receive(boost::asio::buffer(__bufRecieve, BUF_SIZE),
			boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return; //not full Json
	}
	try
	{
		__parser.finish();
		__bufJsonRecieve = __parser.release();
		__parser.reset();
	}
	catch (exception& e)
	{
		__log->writeLog(2, "DataBase", "Error_failed_to_read_request");
		__log->writeTempLog(2, "DataBase", "Error_failed_to_read_request");
		cerr << e.what() << endl;
		__socket->async_receive(boost::asio::buffer(__bufRecieve, BUF_SIZE),
			boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		__errorWhat.clear();
		__errorWhat = e.what();
		return; //exception
	}

	string command = __checkCommand(__bufRecieve, bytesRecieve);
	if (command == "ping")
	{
		__log->writeTempLog(0, "DataBase", "Make_ping_response");
		__makePing();
	}
	else if (command == "disconnect")
	{
		__log->writeTempLog(0, "DataBase", "Make_disconnect_response");
		__makeDisconnect();
	}
	else if (command == "select")
	{
		__log->writeTempLog(0, "DataBase", "Make_query_response");
		__makeQuery();
	}
	else if (command == "error")
	{
		__log->writeTempLog(0, "DataBase", "Make_error_response");
		__makeError();
	}
	__log->writeLog(0, "DataBase", "Send_response_command");
	__log->writeTempLog(0, "DataBase", "Send_response_command");
	__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	if (__flagWrongConnect)
	{
		stop();
		__flagWrongConnect = false;
	}
}

void DataBase::__sendCommand(const boost::system::error_code& eC, size_t bytesSend)
{
	if (eC)
	{
		__log->writeLog(2, "DataBase", "Error_send_response_command");
		__log->writeTempLog(2, "DataBase", "Error_send_response_command");
		cerr << eC.message() << endl;
		Sleep(2000);
		__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	static size_t tempSend = 0;
	tempSend += bytesSend;
	if (tempSend != __bufSend.size())
	{
		__log->writeTempLog(0, "DataBase", "Not_full_json");
		__socket->async_send(net::buffer(__bufSend.c_str() + tempSend, __bufSend.size() - tempSend), boost::bind(&DataBase::__sendCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	tempSend = 0;
	__bufSend.clear();
	__log->writeTempLog(0, "DataBase", "Recieve_request");
	__socket->async_receive(net::buffer(__bufRecieve, BUF_SIZE), boost::bind(&DataBase::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

string DataBase::__checkCommand(char* __bufRecieve, size_t bytesRecieve)
{
		if (__bufJsonRecieve.at("target").as_string() == "ping")
		{
			return "ping";
		}
		else if (__bufJsonRecieve.at("target").as_string() == "disconnect")
		{
			return "disconnect";
		}
		else if (__bufJsonRecieve.at("target").as_string() == "DB_query")
		{
			//return method to string
		}
		else
		{
			__log->writeTempLog(2, "DataBase", "No_command");
			__errorWhat.clear();
			__errorWhat = "non-existent command";
			return "error";//no command
		}
}

void DataBase::__makePing()
{
	__bufSend.clear();
	__bufSend = boost::json::serialize(json_formatter::database::response::ping("Data_Base"));
}

void DataBase::__makeDisconnect()
{
	__bufSend.clear();
	__bufSend = boost::json::serialize(json_formatter::database::response::disconnect("Data_Base"));
	__flagWrongConnect = true;
}

void DataBase::__makeQuery()
{
	boost::json::array jsonFields = __bufJsonRecieve.at("request").at("fields").as_array();
	vector<string> fields;
	for (size_t i = 0; i < jsonFields.size(); i++)
	{
		fields.emplace_back(jsonFields[i].as_string());
	}
	__query.clear();

	map<string, vector<string>> bufResponse;
	__query = boost::json::serialize(__bufJsonRecieve.at("request").at("query"));

	int exit = sqlite3_open(DB_WAY, &__dB);
	int flag = sqlite3_exec(__dB, __query.c_str(), __connection, (void*)&bufResponse, NULL);
	sqlite3_close(__dB);

	map<string, vector<string>> response;
	map<string, vector<string>>::iterator it;

	for (size_t i = 0; i < fields.size(); i++)
	{
		it = bufResponse.find(fields[i]);
		if (it != bufResponse.end())
		{
			vector<string> b = it->second;
			response[fields[i]] = b;
		}
	}

	__bufSend.clear();
	__bufSend =boost::json::serialize(json_formatter::database::response::query("Data_base", json_formatter::database::QUERY_METHOD::SELECT, response));



}
void DataBase::__makeError()
{
	__bufSend.clear();
	//__bufSend = boost::json::serialize(json_formatter::database::) //нет команды
}

void DataBase::__checkConnect(string login, string password)
{
	int exit = sqlite3_open(DB_WAY, &__dB);
	__query.clear();
	__query = "SELECT * FROM Accounts WHERE Login = '" + login + "'";
	__bufSend.clear();
	map<string, vector<string>> __Answer;
	map<string, vector<string>> it;
	int flag = sqlite3_exec(__dB, __query.c_str(), __connection,(void*)&__Answer,NULL);
	if (flag != SQLITE_OK)
	{
		__bufSend = boost::json::serialize(json_formatter::database::response::connect("Data_Base", json_formatter::ERROR_CODE::CONNECT, "User not found"));
		__flagWrongConnect = true;
	}
	else
	{
		vector<string> __password = __Answer.at(login);
		if (__password[0] == password)
		{
			__bufSend = boost::json::serialize(json_formatter::database::response::connect("Data_Base"));
		}
		else
		{
			__bufSend = boost::json::serialize(json_formatter::database::response::connect("Data_Base", json_formatter::ERROR_CODE::CONNECT, "Password mismatch"));
			__flagWrongConnect = true;
		}
	}
	sqlite3_close(__dB);
}

int DataBase::__connection(void* Answer, int argc, char** argv, char** azColName)
{
	map<string, vector<string>>* bufMap = ((map<string, vector<string>>*)Answer);
	for (size_t i = 0; i < argc; i++)
	{
		string bufAnswer = argv[i];
		string colName = azColName[i];
		vector<string> bufVec = bufMap->at(bufAnswer);
		bufVec.push_back(bufAnswer);
		(*bufMap)[colName] = bufVec;
	}

	return 0;
}

Server::Server(net::io_context& io_context, string nameConfigFile)
{
	__logServer = make_shared<Log>("", "./", "DataBase");
	__config = make_shared<Config>(__logServer, "./", "",nameConfigFile);
	__config->readConfig();
	__configInfo = __config->getConfigInfo();
	if (__configInfo.size() == 0)
	{
		__logServer->writeLog(1, "DataBase", "Config_File_not_open");
		return;
	}
	else if (__configInfo.size() < CONFIG_NUM_FIELDS)
	{
		__logServer->writeLog(1, "DataBase", "Config_File_not_full");
	}
	try
	{
		for (size_t i = 0, length = __configInfo.size(); i < length; i++)
		{
			__configInfo.at(CONFIG_FIELDS.at(i));
		}
	}
	catch (exception& e) 
	{
		__logServer->writeLog(1, "DataBase", e.what());
		return;
	}
	string port = __configInfo.at("port");
	__acceptor = make_shared<tcp::acceptor>(io_context, tcp::endpoint(tcp::v4(), stoi(port)));
	
	__doAccept();
}

void Server::__doAccept()
{
	__acceptor->async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				?shared_ptr<tcp::socket> sock = &socket
				make_shared<DataBase>(__logServer, sock);
			}
			__doAccept();
		});
		
}
