#include "GlobalModules/ClientDB/ClientDB.hpp"
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
	string __name = "ClientDB";
	string __bDLog = "1";
	string __bDPas = "1";
	queue<string> tables;
	tables.push("MarussiaStation"); tables.push("House"); tables.push("StaticPhrases");
	queue<vector<string>> fields;
	vector<string> fields_marussia = { "ApplicationId", "ComplexId", "HouseNum" };
	vector<string> fields_house = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> fields_phrases = { "ComplexId", "HouseNumber", "KeyWords", "Response" };
	fields.push(fields_marussia); fields.push(fields_house); fields.push(fields_phrases);

	shared_ptr<tcp::socket> socket = make_shared<tcp::socket>(ioc);
	shared_ptr<ClientDB> Client = make_shared<ClientDB>(ip, port, __bDLog, __bDPas, "Worker_Test", socket);
	Client->setQuerys(tables, fields);
	Client->start();
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