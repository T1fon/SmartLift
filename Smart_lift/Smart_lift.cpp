#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <boost/thread.hpp>
#include <iostream>

#include "Modules/Server.hpp"

using namespace std;

int main() 
{
    std::cout << "HIHIHIH" << std::endl;  

	try
	{
		string add = "0.0.0.0";
		string p = "80";
		auto const address = net::ip::make_address(add);
		unsigned short port = static_cast<unsigned short>(atoi("80"));

		net::io_context ioc{1};

		tcp::acceptor acceptor{ioc, { address, port }};
		tcp::socket socket{ioc};
		HttpServer(acceptor, socket);

		ioc.run();
	}
	catch (exception const& e)
	{
		cerr << "Error" << e.what() << endl;
		return EXIT_FAILURE;
	}
    
}
