#pragma once

#include "Modules/Worker/Worker.hpp"

class ServerWorker : public enable_shared_from_this<ServerWorker>
{
private:
	shared_ptr<net::io_context> __ioc;
	short __countThreads = 2;
	shared_ptr<Server> __server;
public:
	ServerWorker()
	{
		__ioc = make_shared<boost::asio::io_context>(__countThreads);
		__server = make_shared<Server>(__ioc, "");
	}
	~ServerWorker()
	{
		stop();
	}
	void start()
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
	void stop()
	{
		__server->stop();
	}

};