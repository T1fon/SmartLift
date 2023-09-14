#include "Resurrector.hpp"

Resurrector::Resurrector()
{
	__log = make_shared<Log>("Log/", "./", "Resurrector");
	__config = make_shared<Config>(__log, "./", "", "");
	__config->readConfig();
}

Resurrector::~Resurrector()
{

}

void Resurrector::stop()
{

}

void Resurrector::start()
{
	__config_info = __config->getConfigInfo();
	if (__config_info.size() == 0)
	{
		return;
	}
	else if (__config_info.size() < CONFIG_NUM_FIELDS)
	{
	}
	try
	{
		for (size_t i = 0, length = __config_info.size() - 1; i < length; i++)
		{
			__config_info.at(CONFIG_FIELDS.at(i));
		}
	}
	catch (exception& e)
	{
		__log->writeLog(1, "Resurrector", e.what());
		cerr << "error read" << endl;
		return;
	}

	try
	{
		__count_threads = stoi(__config_info.at("Count_threads"));
		__time_check_s = stoi(__config_info.at("Time_check"));
		__time_reset_h = stoi(__config_info.at("Time_reset"));
	}
	catch (exception& e)
	{
		cerr << "Resurrector not read timers or threads " << e.what() << endl;
	}
	cerr << 100 << endl;
	__ioc = make_shared<boost::asio::io_context>(__count_threads);
	__timer_check = make_shared<net::deadline_timer>(*__ioc);
	__timer_reset = make_shared<net::deadline_timer>(*__ioc);
	cerr << 1 << endl;
	__timer_check->expires_from_now(boost::posix_time::seconds(__time_check_s));
	cerr << 101 << endl;
	__timer_reset->expires_from_now(boost::posix_time::hours(__time_reset_h));
	__timer_check->async_wait(boost::bind(&Resurrector::__resetTimerCheck, this));
	__timer_reset->async_wait(boost::bind(&Resurrector::__resetTimerReset, this));
	cerr << 2 << endl;
	vector<std::thread> v;
	v.reserve(__count_threads - 1);
	for (auto i = __count_threads - 1; i > 0; --i)
		v.emplace_back(
			[this]
			{
				__ioc->run();
			});
	__ioc->run();
}

void Resurrector::__resetTimerCheck()
{
	cerr << 3 << endl;
	__checkWorking();
	__timer_check->expires_from_now(boost::posix_time::seconds(__time_check_s));
	__timer_check->async_wait(boost::bind(&Resurrector::__resetTimerCheck, shared_from_this()));
}

void Resurrector::__resetTimerReset()
{
	cerr << 4 << endl;
	__resetProgramms();
	__timer_reset->expires_from_now(boost::posix_time::hours(__time_reset_h));
	__timer_reset->async_wait(boost::bind(&Resurrector::__resetTimerReset, shared_from_this()));
}

void Resurrector::__checkWorking()
{
	cerr << 5 << endl;
	__dir_entity = NULL;
	__dir_proc = NULL;

	__dir_proc = opendir(PROC_DIRECTORY);
	cerr << 6 << endl;
	if (__dir_proc == NULL)
	{
		cerr << "Not open " <<  PROC_DIRECTORY << endl;
		return;
	}
	while ((__dir_entity = readdir(__dir_proc)) != NULL)
	{
		cerr << 6.1 << endl;
		if (!isdigit(*__dir_entity->d_name))
		{
			continue;
		}
		cerr << 6.2 << endl;
		cerr << __dir_entity->d_name << endl;
		string pid, name;
		char arg1[30];
		sprintf(arg1, "/proc/%d/stat", __dir_entity->d_name);
		ifstream stat_stream(arg1, ios_base::in);
		stat_stream >> pid >> name;
		cerr << pid << " " << name << endl;
		cerr << 6.3 << endl;
		if (name == "Worker")
		{
			__flag_worker = true;
		}
		else if (name == "ServerDB")
		{
			__flag_db = true;
		}
		else if (name == "MainServer")
		{
			__flag_main_server = true;
		}
		else if (name == "MQTT_Worker")
		{
			__flag_mqtt_worker = true;
		}
	}
	closedir(__dir_proc);
	cerr << 7 << endl;
	if (__flag_worker == false)
	{
		try
		{
			string boofWay = __config_info.at("Worker");
			string way = " nohup ./" + boofWay + "Worker -cf "+ boofWay + " &";
			cerr << "way" << way << endl;
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "open Worker " << e.what() << endl;
		}
	}
	else if (__flag_db == false)
	{
		try
		{
			string boofWay = __config_info.at("ServerDB");
			string way = " nohup ./" + boofWay + "ServerDB -cf "+ boofWay + " &";
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "open ServerDB " << e.what() << endl;
		}
	}
	else if (__flag_main_server == false)
	{
		try
		{
			string boofWay = __config_info.at("MainServer");
			string way = " nohup ./" + boofWay + "MainServer -cf "+ boofWay + " &";
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "open ServerDB " << e.what() << endl;
		}
	}
	else if (__flag_main_server == false)
	{
		try
		{
			string boofWay = __config_info.at("MQTT_Worker");
			string way = " nohup ./" + boofWay + "MQTT_Worker -cf " + boofWay +" &";
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "open ServerDB " << e.what() << endl;
		}
	}
	cerr << 8 << endl;
}

void Resurrector::__resetProgramms()
{
	cerr << 9 << endl;
	__dir_entity = NULL;
	__dir_proc = NULL;

	__dir_proc = opendir(PROC_DIRECTORY);
	if (__dir_proc == NULL)
	{
		cerr << "Not open " <<  PROC_DIRECTORY << endl;
		return;
	}
	cerr << 10 << endl;
	while ((__dir_entity = readdir(__dir_proc)) != NULL)
	{
				if (!isdigit(*__dir_entity->d_name))
		{
			continue;
		}
		errno = 0;
		cerr << __dir_entity->d_name << endl;
		string pidd, name;
		char arg1[20];
		sprintf(arg1, "/proc/%d/stat", __dir_entity->d_name);
		ifstream stat_stream(arg1, ios_base::in);
		stat_stream >> pidd >> name;
		cerr << pidd << " " << name << endl;
		for(size_t i = 0; i < __fields.size(); i++)
		{
			if(name == __fields[i])
			{
				pid_t pid = stoi(pidd);
				int  killReturn = kill(pid, SIGKILL);
				if (killReturn == -1)
				{
					if (errno == ESRCH)      // pid does not exist
					{
						cerr << "Group does not exist!" << endl;
					}
					else if (errno == EPERM) // No permission to send signal
					{
						cerr << "No permission to send signal!" << endl;
					}
					else
						cerr << "Signal sent. All Ok!" << endl;
				}
			}
		}
	}
	closedir(__dir_proc);
	cerr << 11 << endl;
	for (size_t i = 0; i < __fields.size(); i++)
	{
		try
		{
			string boofWay = __config_info.at(__fields[i]);
			string way = " nohup ./" + boofWay + __fields[i] + "-cf " + boofWay + " &";
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "Reset " << e.what() << endl;
		}
	}
	cerr << 12 << endl;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
    string config_file_way = "";
    string config_file_name = "";
	
    for (int i = 1; i < argc; i++) {
        string flags = argv[i];
        if (flags == "-cf" || flags == "--config_file") {
            config_file_way = argv[++i];
        }
        else if (flags == "-cf_n" || flags == "--config_file_name")
        {
            config_file_name = argv[++i];
        }
    }
	Resurrector res;
	res.start();
}