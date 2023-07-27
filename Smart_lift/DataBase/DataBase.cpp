#include "boost/asio.hpp"
#include<iostream>
#include "Modules/Server/DataBaseServer.hpp"

using namespace std;

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	string configFile = "";
	for (size_t i = 0; i < argc; i++)
	{
		string flags = argv[i];
		{
			if (flags == "-cf" || flags == "--config_file")
			{
				configFile = argv[++i];
			}
		}
	}
	boost::asio::io_context	ioc;
	Server s(ioc, "");
	ioc.run();
}
