#include "Log.hpp"

Log::Log(string way, string rootDirectory, string nameClass)
{
	__root_directory = rootDirectory;
	__name_file = nameClass;
	__way = way;
	__date = getDate();
	__num_file = 1;
	setFinalPath();
	__num_massive = 2;
}



void Log::writeLog(int error, string clas, string message)
{
	bool flagFile = __checkFile();
	if (flagFile == false)
	{
		__writeLogToFile(clas, message, error);
	}
	else
	{
		string buffDate = getDate();
		streamsize fileSize = getFileSize();
		if (fileSize >= 204800)
		{
			if (__date == buffDate)
			{
				__num_file++;
				setFinalPath();
			}
			else
			{
				__date = buffDate;
				__num_file = 1;
				setFinalPath();
			}
		}
		else if (buffDate != __date)
		{
			__date = buffDate;
			__num_file = 1;
			setFinalPath();
		}
		else
		{
			setFinalPath();
		}
		__writeLogToFile(clas, message, error);
	}
}

void Log::__writeLogToFile(string clas, string message, int error)
{
	ofstream ost(__final_path, ios::app);
	string buffLog = "(" + __date + ")_" + clas + "_" + "\"" + message + "\"";
	if (error != 0)
	{
		buffLog = buffLog + "_" + to_string(error);
	}
	buffLog = buffLog + "\n";
	ost << buffLog;
	ost.close();
}

string Log::getDate()
{
	time_t curtime;
	struct tm* loctime;
	char buffer[12];
	time(&curtime);
	loctime = localtime(&curtime);
	strftime(buffer, 12, "%d_%m_%Y", loctime);
	string time = buffer;
	return time;
}

bool Log::__checkFile()
{
	ifstream fin(__final_path);
	if (fin.is_open())
	{
		return true;
		fin.close();
	}
	else
	{
		return false;
	}
}

streamsize Log::getFileSize()
{
	fstream file(__final_path, fstream::in);
	file.seekg(0, ios::end);
	streamsize fileSize = file.tellg();
	file.close();
	return fileSize;

}

void Log::setFinalPath()
{
	__final_path = __root_directory + __way + __name_file + "_(" + __date + ")_" + to_string(__num_file) + ".log";
}

void Log::writeTempLog(int error, string clas, string message)
{
	string buffLog = "(" + __date + ")_" + clas + "_" + "\"" + message + "\"";
	if (error != 0)
	{
		buffLog = buffLog + "_" + to_string(error);
	}
	switch (__num_massive)
	{
	case 2:
	{
		__temporary_log[__num_massive] = buffLog;
		__num_massive = 1;
		break;
	}
	case 1:
	{
		__temporary_log[__num_massive] = __temporary_log[__num_massive + 1];
		__temporary_log[__num_massive + 1] = buffLog;
		__num_massive = 0;
		break;
	}
	case 0:
	{
		for (int i = 0; i < 2; i++)
		{
			__temporary_log[i] = __temporary_log[i + 1];
		}
		__num_massive = 2;
		__temporary_log[__num_massive] = buffLog;
		break;
	}
	default:
		break;
	}
}
