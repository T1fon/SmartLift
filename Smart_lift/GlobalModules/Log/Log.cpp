#include "Log.hpp"

Log::Log(string way, string rootDirectory, string nameClass)
{
	__rootDirectory = rootDirectory;
	__nameFile = nameClass;
	__way = way;
	__date = getDate();
	__numFile = 1;
	setFinalPath();
	__numMassive = 2;
}



void Log::writeLog(int error, string clas, string message)
{
	bool flagFile = checkFile();
	if (flagFile == false)
	{
		writeLogToFile(clas, message);
	}
	else
	{
		string buffDate = getDate();
		streamsize fileSize = getFileSize();
		if (fileSize >= 204800)
		{
			if (__date == buffDate)
			{
				__numFile++;
				setFinalPath();
			}
			else
			{
				__date = buffDate;
				__numFile = 1;
				setFinalPath();
			}
		}
		else if (buffDate != __date)
		{
			__date = buffDate;
			__numFile = 1;
			setFinalPath();
		}
		else
		{
			setFinalPath();
		}
		writeLogToFile(clas, message);
	}
}

void Log::writeLogToFile(string clas, string message)
{
	ofstream ost(__finalPath, ios::app);
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

bool Log::checkFile()
{
	ifstream fin(__finalPath);
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
	fstream file(__finalPath, fstream::in);
	file.seekg(0, ios::end);
	streamsize fileSize = file.tellg();
	file.close();
	return fileSize;

}

void Log::setFinalPath()
{
	__finalPath = __rootDirectory + __way + __nameFile + "_(" + __date + ")_" + to_string(__numFile) + ".log";
}

void Log::writeTempLog(int error, string clas, string message)
{
	string buffLog = "(" + __date + ")_" + clas + "_" + "\"" + message + "\"";
	if (error != 0)
	{
		buffLog = buffLog + "_" + to_string(error);
	}
	switch (__numMassive)
	{
	case 2:
	{
		__temporaryLog[__numMassive] = buffLog;
		__numMassive = 1;
		break;
	}
	case 1:
	{
		__temporaryLog[__numMassive] = __temporaryLog[__numMassive + 1];
		__temporaryLog[__numMassive + 1] = buffLog;
		__numMassive = 0;
		break;
	}
	case 0:
	{
		for (int i = 0; i < 2; i++)
		{
			__temporaryLog[i] = __temporaryLog[i + 1];
		}
		__numMassive = 2;
		__temporaryLog[__numMassive] = buffLog;
		break;
	}
	default:
		break;
	}
}
