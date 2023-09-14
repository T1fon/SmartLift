#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <cerrno>

#include "../GlobalModules/Config/Config.hpp"
#include "../GlobalModules/Log/Log.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define PROC_DIRECTORY "/proc/"

using namespace std;
namespace net = boost::asio;

class Resurrector : public enable_shared_from_this<Resurrector>
{
private:

	shared_ptr<net::io_context> __ioc;
	short __count_threads;
	short __time_check_s;
	short __time_reset_h;
	shared_ptr<Config> __config;
	shared_ptr<Log> __log;
	shared_ptr<net::deadline_timer> __timer_check;
	shared_ptr<net::deadline_timer> __timer_reset;
	map<string, string> __config_info;
	static const int CONFIG_NUM_FIELDS = 5;
	struct dirent* __dir_entity;
	DIR* __dir_proc;
	bool __flag_worker;
	bool __flag_db;
	bool __flag_main_server;
	bool __flag_mqtt_worker;
	vector<string> CONFIG_FIELDS = { "Time_check", "Time_reset", "ServerDB", "MainServer", "Worker", "MQTT_Worker", "Count_threads"};
	vector<string> __fields = {"Worker"};

	void __checkWorking();
	void __resetProgramms();
	void __resetTimerCheck();
	void __resetTimerReset();

public:
	Resurrector();
	~Resurrector();
	void start();
	void stop();
};