#include "Smart_lift.h"
#include <iostream>

using namespace std;

int main() 
{
	boost::asio::io_context ioc;
    std::cout << "HIHIHIH" << std::endl;  
	string ip = "127.0.0.1";
	string port = "80";
	queue<string> que;
	string request = boost::json::serialize(json_formatter::database::request::connect("Worker", "1", "1"));
	make_shared<Client>(ip, port, ioc, que)->start();
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