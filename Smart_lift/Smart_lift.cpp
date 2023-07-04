#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <boost/thread.hpp>
#include <iostream>

#include "Modules/Server.hpp"
#include "Modules/Client/Client.hpp"

using namespace std;

int main() 
{
    std::cout << "HIHIHIH" << std::endl;  


	try
	{
		string add = "0.0.0.0";
		string p = "80";
		const char* addr = "127.0.0.1";
		const char* por = "80";
		string doocRoot = ".";
		const char* target = "/";
		auto const address = net::ip::make_address(add);
		auto const port = static_cast<unsigned short>(atoi("80"));
		auto const docRoot = make_shared<string>(doocRoot);
		auto const threads = max<int>(1, 2);

		net::io_context ioc{threads};
		make_shared<Listener>(ioc, tcp::endpoint{address, port}, docRoot)->Run();
		vector<thread> v;
		v.reserve(threads - 1);
		for (auto i = threads - 1; i > 0; i--)
		{
			v.emplace_back([&ioc]
				{
					ioc.run();
				}
			);
		}
		make_shared<Client>(ioc)->RunClient(addr, por, target, 11);
		ioc.run();
		//make_shared<Client>(ioc)->RunClient(addr, por, target , 1.0);
	}
	catch (exception const& e)
	{
		cerr << "Error" << e.what() << endl;
		return EXIT_FAILURE;
	}
    
}
