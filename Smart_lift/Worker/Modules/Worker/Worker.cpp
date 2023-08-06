/*#include "Worker.hpp"

Worker::Worker(shared_ptr<tcp::socket> socket, map<string, string> confInfo, net::io_context& ioc, shared_ptr<Log> lg) : __socket(move(socket)), __ioc(ioc)
{
	__bufRecive = new char[BUF_RECIVE_SIZE + 1];
	__bufSend = "";
	__bufJsonRecive = {};
	__parser.reset();
	__configInfo = confInfo;
	__timer = make_shared<net::deadline_timer>(__ioc);
	try
	{
		__ipMS = __configInfo.at("Main_server_ip");
		 __portMS = __configInfo.at("Main_server_port");
		 __workerId = __configInfo.at("Id");
		 __ipBd = __configInfo.at("BD_ip");
		 __portBd = __configInfo.at("BD_port");
		 __bDLog = __configInfo.at("BD_login");
		 __bDPas = __configInfo.at("BD_password");
		__socketDB = make_shared<tcp::socket>(__ioc);
		__dBClient = make_shared<ClientDB>(__ipBd, __portBd, __socketDB);
		__mSClient = make_shared<ClientMS>(__ipMS, __portMS, __socket);
	

	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}

void Worker::start()
{
	__timer->expires_from_now(boost::posix_time::hours(24));
	__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
	__mSClient->start();
	__connectToBd();
	__connectToMS();
}

void Worker::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
}

void Worker::__resetTimer()
{
	__connectToBd();
	__timer->expires_from_now(boost::posix_time::hours(24));
	__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
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
	__bufSend = boost::json::serialize(json_formatter::worker::request::connect(__name,__id));
	__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2));
}

void Worker::__recieveConnectToMS(const boost::system::error_code& eC, size_t bytesSend)
{
	static size_t tempBytesSend = 0;
	/*if (__sendCheck(bytesSend, tempBytesSend, boost::bind(&Client::__sendConnect, shared_from_this(), boost::lambda2::_1, boost::lambda2::_2)) == __CHECK_STATUS::FAIL) {
		return;
	}
	tempBytesSend += bytesSend;
	if (__bufSend.size() != tempBytesSend) {
		__socket->async_send(boost::asio::buffer(__bufSend.c_str() + tempBytesSend, (__bufSend.size() - tempBytesSend)),
			boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	tempBytesSend = 0;
	__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void Worker::__connectToBd()
{
		string Connect = boost::json::serialize(json_formatter::database::request::connect(__name, __bDLog, __bDPas));
		string selectMarussiaStation = boost::json::serialize(json_formatter::database::request::query(__name,
			json_formatter::database::QUERY_METHOD::SELECT, __marussiaStationFields, "SELECT * FROM MarussiaStation"));
		string selectHouse = boost::json::serialize(json_formatter::database::request::query(__name,
			json_formatter::database::QUERY_METHOD::SELECT, __houseFields, "SELECT * FROM House"));
		string selectStaticPhrases = boost::json::serialize(json_formatter::database::request::query(__name,
			json_formatter::database::QUERY_METHOD::SELECT, __staticPhrasesFields, "SELECT * FROM StaticPhrases"));
		string disconnect = boost::json::serialize(json_formatter::database::request::disconnect(__name));
		queue<string> requestBody;
		requestBody.push(Connect);
		requestBody.push(selectMarussiaStation);
		requestBody.push(selectHouse);
		requestBody.push(selectStaticPhrases);
		requestBody.push(disconnect);

		__dBClient->setQuery(requestBody);
		__dBClient->start();///вопрос, успеет ли, или надо поставить sleep
		__dBInfo = __dBClient->getRespData();
		if (__dBInfo.size() < 3)
		{

		}
		__dBClient->stop();
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
	boost::json::value target = __bufJsonRecive.at("target");
	boost::json::value status = __bufJsonRecive.at("response").at("status");

	if (target == "ping")
	{
		string ping = boost::json::serialize(json_formatter::worker::response::ping(__name));
		__bufSend = boost::json::serialize(json_formatter::worker::response::ping(__name));
	}
	else if (target == "disconnect")
	{
		string respDisconnect = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
		__bufSend = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
	}
	else if (target == "marussia_station_request")
	{
		__analizeRequest();
	}
	else if (target == "connect")
	{

		 if (status == "success")
		 {
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&Worker::__waitCommand, shared_from_this(),
				boost::placeholders::_1, boost::placeholders::_2));
		 }
		 else if (status == "fail")
		 {
			 boost::json::value fail = __bufJsonRecive.at("response").at("message");
			 cerr << "error connect " << fail << endl;
			 __connectToMS();
		 }
	}

	//делаем таймер на работу бд
	__socket->async_send(net::buffer(__bufSend, __bufSend.size()), boost::bind(&Worker::__recieveConnectToMS, shared_from_this(),
		boost::placeholders::_1, boost::placeholders::_2));
}



void Worker::__analizeRequest()
{
	///___marussia station House number and complex id___///
	string appId = boost::json::serialize(__bufJsonRecive.at("application").at("application_id"));
	map<string, vector<string>> oneTable = __dBInfo.at("MarussiaStation");
	vector<string> bufVec = __dBInfo.at("MarussiaStation").at("ApplicationId");
	vector<string> houseVec = __dBInfo.at("MarussiaStation").at("HouseNum");
	vector<string> compVec = __dBInfo.at("MarussiaStation").at("ComplexId");
	vector<string> liftVec = __dBInfo.at("MarussiaStation").at("LiftBlockId");

	string numHouse = "-1";
	string compId = "-1";
	string liftblock = "";
	string response = "";

	for (size_t i = 0; i < bufVec.size(); i++)
	{
		if (appId == bufVec[i])
		{
			numHouse = houseVec[i];
			compId = compVec[i];
			liftblock = liftVec[i];
			break;
		}
	}

	///___search for mqtt_____/////
	boost::json::array ar = __bufJsonRecive.at("request").at("nlu").at("tokens").as_array();
	vector<string> searchMqtt;
	bool flagMqtt = false;
	for (size_t i = 0; i < ar.size(); i++)
	{
		searchMqtt.push_back(boost::json::serialize(ar[i]));
		if (ar[i].as_string() == "этаж" || ar[i].as_string() == "подъем" || ar[i].as_string() == "спуск" || ar[i].as_string() == "подними" || ar[i].as_string() == "опусти")
		{
			flagMqtt = true;
		}
	}
	if (flagMqtt)
	{
		oneTable.clear();
		oneTable = __dBInfo.at("House");
		bufVec.clear(); houseVec.clear(); compVec.clear();
		houseVec = __dBInfo.at("House").at("HouseNum");
		compVec = __dBInfo.at("House").at("ComplexId");
		vector<string> topFloor = __dBInfo.at("House").at("TopFloor");
		vector<string> botFl = __dBInfo.at("House").at("BottomFloor");
		vector<string> nullFl = __dBInfo.at("House").at("NullFloor");
		string bufNum = "";
		int floor;
		for (size_t i = 0; i < __keyRoots.size(); i++)
		{
			for (size_t j = 0; j < searchMqtt.size(); j++)
			{
				if (searchMqtt[j].find(__keyRoots[i]) != searchMqtt[j].npos)
				{
					bufNum += searchMqtt[j];
				}
			}
		}
		int numFloor = __numRoots.at(bufNum);

		for (size_t i = 0; i < houseVec.size(); i++)
		{
			if (houseVec[i] == numHouse && compVec[i] == compId)
			{
				if (topFloor[i] == to_string(numFloor) || botFl[i] == to_string(numFloor) || nullFl[i] == to_string(numFloor))
				{
					response = "перемещаю вас на " + bufNum + "этаж";
					break;
				}
			}
		}
		__bufSend = boost::json::serialize(json_formatter::worker::response::marussia_mqtt_message(__name, appId, __getRespToMS(response),liftblock,numFloor));
	}
	else
	{
		///_____static phrases response static phrase___///

		string command = boost::json::serialize(__bufJsonRecive.at("request").at("command"));
		oneTable.clear();
		oneTable = __dBInfo.at("StaticPhrases");
		bufVec.clear(); houseVec.clear(); compVec.clear();
		bufVec = __dBInfo.at("StaticPhrases").at("KeyWords");
		houseVec = __dBInfo.at("StaticPhrases").at("HouseNumber");
		compVec = __dBInfo.at("StaticPhrases").at("ComplexId");

		vector<string> resp = __dBInfo.at("StaticPhrases").at("Response");

		response.clear();

		for (size_t i = 0; i < bufVec.size(); i++)
		{
			if (command == bufVec[i])
			{
				if (numHouse == houseVec[i] && compId == compVec[i])
				{
					response = resp[i];
					break;
				}
			}
		}
		if (response != "")
		{
			__bufSend = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, appId, __getRespToMS(response)));
		}
		else
		{
			response = "Извините, я не знаю такой комманды, попробуйте перефразировать";
			__bufSend = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, appId, __getRespToMS(response)));
		}
	}

}

boost::json::object Worker::__getRespToMS(string respText)
{
	return boost::json::object({ {"response",{
												{"text", respText},
												{"tts", respText},
												{"end_session", "false"}
											}}});
}

///--------------------------------------------------------------------------///
ClientDB::ClientDB(string ipBD, string portBD, shared_ptr<tcp::socket> socket)
{
	__endPoint = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ipBD), stoi(portBD)));
	__socket = socket;
	__bufRecive = new char[BUF_RECIVE_SIZE + 1];
	fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
	__bufJsonRecive = {};
	__parser.reset();
}

void ClientDB::start()
{
	__socket->async_connect(*__endPoint, boost::bind(&ClientDB::__checkConnect, shared_from_this(), boost::placeholders::_1));
}

void ClientDB::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
}

ClientDB::~ClientDB()
{
	this->stop();
	delete[] __bufRecive;
}

void ClientDB::__checkConnect(const boost::system::error_code& eC)
{
	if (eC)
	{
		cerr << eC.what() << endl;
		Sleep(2000);
		this->stop();
		this->start();
		return;
	}
	if (!__queueToSend.empty())
	{
		__bufQueueString = __queueToSend.front();
		__queueToSend.pop();
		__socket->async_send(boost::asio::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}
	else
	{
		this->stop();

	}
}

void ClientDB::setQuery(queue<string> queue)
{
	__queueToSend = queue;
}

map<string, map<string, vector<string>>> ClientDB::getRespData()
{
	return __respData;
}

ClientDB::__CHECK_STATUS ClientDB::__sendCheck(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler)
{
	temp_send_byte += count_send_byte;
	if (__bufQueueString.size() != temp_send_byte) {
		__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + temp_send_byte, (__bufQueueString.size() - temp_send_byte)), handler);
		return __CHECK_STATUS::FAIL;
	}
	return __CHECK_STATUS::SUCCESS;
}

ClientDB::__CHECK_STATUS ClientDB::__reciveCheck(const size_t& count_recive_byte, __handler_t&& handler)
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

void ClientDB::__sendConnect(const boost::system::error_code& eC, size_t bytesSend)
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
	}
	tempBytesSend += bytesSend;
	if (__bufQueueString.size() != tempBytesSend) {
		__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + tempBytesSend, (__bufQueueString.size() - tempBytesSend)),
			boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	tempBytesSend = 0;
	__bufQueueString.clear();
	__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void ClientDB::__reciveCommand(const boost::system::error_code& eC, size_t bytesRecive)
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
	}
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
		__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(),
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

void ClientDB::__commandAnalize()
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
				__respError.clear();
				__respError = __bufJsonRecive.as_string();
				this->stop();
				return;
			}
			else
			{
				if (!__queueToSend.empty())
				{
					__bufQueueString = __queueToSend.front();
					__queueToSend.pop();
					__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
				}
				else
				{
					this->stop();
					return;
				}
				
			}
		}
	}
	else if (target == "disconnect")
	{
		if (status == "success")
		{
			this->stop();
			return;
		}
		else
		{
			__respError.clear();
			__respError = __bufJsonRecive.as_string();
			__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientDB::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
	}
	else if (target == "db_query")
	{
		map<string, vector<string>> bufRespMap;
		vector<string> valueMap;
		if (__bufJsonRecive.at("response").at(__marussiaStationFields[0]) != "")
		{
			for (size_t i = 0; i < __marussiaStationFields.size(); i++)
			{
				valueMap.clear();
				boost::json::array valueJson = __bufJsonRecive.at("response").at(__marussiaStationFields[i]).as_array();
				for (size_t j = 0; j < valueJson.size(); j++)
				{
					valueMap.push_back(boost::json::serialize(valueJson[j]));
				}
				bufRespMap[__marussiaStationFields[i]] = valueMap;
			}
			__respData["marussiaStation"] = bufRespMap;
		}
		else if (__bufJsonRecive.at("response").at(__houseFields[0]) != "")
		{
			for (size_t i = 0; i < __houseFields.size(); i++)
			{
				valueMap.clear();
				boost::json::array valueJson = __bufJsonRecive.at("response").at(__houseFields[i]).as_array();
				for (size_t j = 0; j < valueJson.size(); j++)
				{
					valueMap.push_back(boost::json::serialize(valueJson[j]));
				}
				bufRespMap[__houseFields[i]] = valueMap;
			}
			__respData["houseFields"] = bufRespMap;
		}
		else if (__bufJsonRecive.at("response").at(__staticPhrasesFields[0]) != "")
		{
			for (size_t i = 0; i < __staticPhrasesFields.size(); i++)
			{
				valueMap.clear();
				boost::json::array valueJson = __bufJsonRecive.at("response").at(__staticPhrasesFields[i]).as_array();
				for (size_t j = 0; j < valueJson.size(); j++)
				{
					valueMap.push_back(boost::json::serialize(valueJson[j]));
				}
				bufRespMap[__staticPhrasesFields[i]] = valueMap;
			}
			__respData["staticPhrases"] = bufRespMap;
		}
		if (!__queueToSend.empty())
		{
			__bufQueueString = __queueToSend.front();
			__queueToSend.pop();
			__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientDB::__sendConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		else
		{
			this->stop();
			return;
		}
	}
}


////--------------------------------------------------///

ClientMS::ClientMS(string ipMS, string portMS, shared_ptr<tcp::socket> socket)
{
	__endPoint = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(ipMS), stoi(portMS)));
	__socket = socket;
	__bufRecive = new char[BUF_RECIVE_SIZE + 1];
	fill_n(__bufRecive, BUF_RECIVE_SIZE, 0);
	__bufJsonRecive = {};
	__parser.reset();
}

ClientMS::~ClientMS()
{
	this->stop();
	delete[] __bufRecive;
}

void ClientMS::start()
{
	__socket->async_connect(*__endPoint, boost::bind(&ClientMS::__readFromW, shared_from_this(), boost::placeholders::_1));
}

void ClientMS::stop()
{
	if (__socket->is_open())
	{
		__socket->close();
	}
}

void ClientMS::__readFromW(const boost::system::error_code& eC)
{
	if (eC)
	{
		cerr << eC.what() << endl;
		Sleep(2000);
		this->stop();
		this->start();
		return;
	}
		__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendFromWTMS, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void ClientMS::__sendFromWTMS(const boost::system::error_code& eC, size_t bytesRecive)
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
	}
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
		__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendFromWTMS, shared_from_this(),
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
	__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientMS::__resFromMStW, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void ClientMS::__resFromMStW(const boost::system::error_code& eC, size_t bytesSend)
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
	}
	tempBytesSend += bytesSend;
	if (__bufQueueString.size() != tempBytesSend) {
		__socket->async_send(boost::asio::buffer(__bufQueueString.c_str() + tempBytesSend, (__bufQueueString.size() - tempBytesSend)),
			boost::bind(&ClientMS::__resFromMStW, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		return;
	}
	tempBytesSend = 0;
	__bufQueueString.clear();
	__socket->async_receive(net::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendResTW, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
}

void ClientMS::__sendResTW(const boost::system::error_code& eC, size_t bytesRecive)
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
	}
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
		__socket->async_receive(boost::asio::buffer(__bufRecive, BUF_RECIVE_SIZE), boost::bind(&ClientMS::__sendResTW, shared_from_this(),
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
	__socket->async_send(net::buffer(__bufQueueString, __bufQueueString.size()), boost::bind(&ClientMS::__readFromW, shared_from_this(), boost::placeholders::_1));
}*/
