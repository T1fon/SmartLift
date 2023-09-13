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
		__log->writeLog(1, "Resurrector", "Config_File_not_open");
		return;
	}
	else if (__config_info.size() < CONFIG_NUM_FIELDS)
	{
		__log->writeLog(1, "Resurrector", "Config_File_not_full");
	}
	try
	{
		for (size_t i = 0, length = __config_info.size(); i < length; i++)
		{
			__config_info.at(CONFIG_FIELDS.at(i));
		}
	}
	catch (exception& e)
	{
		__log->writeLog(1, "Resurrector", e.what());
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

	__timer_check->expires_from_now(boost::posix_time::seconds(__time_check_s));
	__timer_check->async_wait(boost::bind(&Resurrector::__resetTimerCheck, shared_from_this()));

	__timer_reset->expires_from_now(boost::posix_time::hours(__time_reset_h));
	__timer_reset->async_wait(boost::bind(&Resurrector::__resetTimerReset, shared_from_this()));
	__ioc = make_shared<boost::asio::io_context>(__count_threads);
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
	__checkWorking();
	__timer_check->expires_from_now(boost::posix_time::seconds(__time_check_s));
	__timer_check->async_wait(boost::bind(&Resurrector::__resetTimerCheck, shared_from_this()));
}

void Resurrector::__resetTimerReset()
{
	__resetProgramms();
	__timer_reset->expires_from_now(boost::posix_time::hours(__time_reset_h));
	__timer_reset->async_wait(boost::bind(&Resurrector::__resetTimerReset, shared_from_this()));
}

void Resurrector::__checkWorking()
{
	__dir_entity = NULL;
	__dir_proc = NULL;

	__dir_proc = opendir(PROC_DIRECTORY);
	if (__dir_proc == NULL)
	{
		cerr << "Not open " <<  PROC_DIRECTORY << endl;
		return;
	}
	while ((__dir_entity = readdir(__dir_proc)) != NULL)
	{
		if (!isdigit(*__dir_entity->d_name))
		{
			continue;
		}
		cerr << __dir_entity->d_name << endl;
		string pid, name;
		char arg1[20];
		sprintf(arg1, "/proc/%d/stat", __dir_entity->d_name);
		ifstream stat_stream(arg1, ios_base::in);
		stat_stream >> pid >> name;
		cerr << pid << " " << name << endl;
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

	if (__flag_worker == false)
	{
		try
		{
			string boofWay = __config_info.at("Worker");
			string way = " nohup ./" + boofWay + "Worker -cf " + boofWay + " &";
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
			string way = " nohup ./" + boofWay + "ServerDB -cf " + boofWay + " &";
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
			string way = " nohup ./" + boofWay + "MainServer -cf " + boofWay + " &";
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
			string way = " nohup ./" + boofWay + "MQTT_Worker -cf " + boofWay + " &";
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "open ServerDB " << e.what() << endl;
		}
	}
}

void Resurrector::__resetProgramms()
{
	__dir_entity = NULL;
	__dir_proc = NULL;

	__dir_proc = opendir(PROC_DIRECTORY);
	if (__dir_proc == NULL)
	{
		cerr << "Not open " <<  PROC_DIRECTORY << endl;
		return;
	}
	while ((__dir_entity = readdir(__dir_proc)) != NULL)
	{
				if (!isdigit(*__dir_entity->d_name))
		{
			continue;
		}
		errno = 0;
		cerr << __dir_entity->d_name << endl;
		string pid, name;
		char arg1[20];
		sprintf(arg1, "/proc/%d/stat", __dir_entity->d_name);
		ifstream stat_stream(arg1, ios_base::in);
		stat_stream >> pid >> name;
		cerr << pid << " " << name << endl;
		for(size_t i = 0; i < __fields.size(); i++)
		{
			if(name == __fields[i])
			{
				pid_t pid = stoi(pid);
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

	for (size_t i = 0; i < __fields.size(); i++)
	{
		try
		{
			string boofWay = __config_info.at(__fields[i]);
			string way = " nohup ./" + boofWay + __fields[i] + " -cf " + boofWay + " &";
			const char* order = way.c_str();
			system(order);
		}
		catch (exception& e)
		{
			cerr << "Reset " << e.what() << endl;
		}
	}
}