#pragma once

#include "Modules/Server/DataBaseServer.hpp"

class ServerDataBase: public enable_shared_from_this<ServerDataBase>
{
private:
	shared_ptr<net::io_context> __ioc;
	short __countThreads = 1;
	shared_ptr<Server> __server;
public:
	ServerDataBase();
	~ServerDataBase();
	void start();
	void stop();

};