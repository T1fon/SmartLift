#include "GlobalModules/Client/Client.hpp"
#include "GlobalModules/JSONFormatter/JSONFormatter.hpp"
#include <iostream>

using namespace std;

int main() 
{
	setlocale(LC_ALL, ".ACP");
	vector<string> __marussiaStationFields = { "ApplicationId", "ComplexId", "HouseNum" };
	vector<string> __houseFields = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> __staticPhrasesFields = { "ComplexId", "HouseNumber", "KeyWords", "Response" };
	boost::asio::io_context ioc;
    std::cout << "HIHIHIH" << std::endl;  
	string ip = "127.0.0.1";
	string port = "80";
	string __name = "Client";
	string __bDLog = "1";
	string __bDPas = "1";
	string Connect = boost::json::serialize(json_formatter::database::request::connect(__name, __bDLog, __bDPas));
	string selectMarussiaStation = boost::json::serialize(json_formatter::database::request::query(__name,
		json_formatter::database::QUERY_METHOD::SELECT, __marussiaStationFields, "SELECT * FROM MarussiaStation"));
	string selectHouse = boost::json::serialize(json_formatter::database::request::query(__name,
		json_formatter::database::QUERY_METHOD::SELECT, __houseFields, "SELECT * FROM House"));
	string selectStaticPhrases = boost::json::serialize(json_formatter::database::request::query(__name,
		json_formatter::database::QUERY_METHOD::SELECT, __staticPhrasesFields, "SELECT * FROM StaticPrhases"));
	string disconnect = boost::json::serialize(json_formatter::database::request::disconnect(__name));
	queue<string> requestBody;
	requestBody.push(Connect);
	requestBody.push(selectMarussiaStation);
	requestBody.push(selectHouse);
	requestBody.push(selectStaticPhrases);
	requestBody.push(disconnect);
	shared_ptr<tcp::socket> socket = make_shared<tcp::socket>(ioc);

	shared_ptr<ClientDB> client = make_shared<ClientDB>(ip, port, socket);
	client->setQuery(requestBody);
	client->start();
	ioc.run();
}
/*int main(int argc, char** argv)
{
	setlocale(LC_ALL, ".ACP");
	try
	{
		auto const threads = max<int>(1, 2);

		net::io_context ioc{threads};
		make_shared<Server>(ioc, "")->Run();
		vector<thread> v;
		v.reserve(threads - 1);
		string ip = "127.0.0.1";
		string port = "80";
		queue<string> que;
		string request = boost::json::serialize(json_formatter::database::request::connect("Worker", "1", "1"));
		que.push(request);
		make_shared<Server>(ioc, "")->Run();
		for (auto i = threads - 1; i > 0; i--)
		{
			v.emplace_back([&ioc]
				{
					ioc.run();
				}
			);
		}
		//make_shared<Client>(ip, port, ioc, que)->start();
		ioc.run();
	}
	catch (exception const& e)
	{
		cerr << "Error" << e.what() << endl;
		return EXIT_FAILURE;
	}
}*/