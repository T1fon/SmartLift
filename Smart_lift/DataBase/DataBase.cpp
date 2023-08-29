/*#include "DataBase.hpp"


ServerDataBase::ServerDataBase()
{
	__ioc =  make_shared<boost::asio::io_context>(__countThreads);
	__server = make_shared<Server>(__ioc, "");
}

ServerDataBase::~ServerDataBase()
{
	stop();
}

void ServerDataBase::start()
{
	__server->run();

	std::vector<std::thread> v;
	v.reserve(__countThreads - 1);
	for (auto i = __countThreads - 1; i > 0; --i)
		v.emplace_back(
			[this]
			{
				__ioc->run();
			});
	__ioc->run();
}

void ServerDataBase::stop()
{
	__server->stop();
}
*/
