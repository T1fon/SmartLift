#include "Log.hpp"

Log::Log(string bufWay, string name) : __bufWay(bufWay), __nameFile(name)
{
	setBufWay(bufWay);//указать отностительный путь в папку записи логов опуская Smart_lift(DataBase\\Log\\)
	setNameFile(name);//указать название программы(Database)
	__way = __way + bufWay;
	__date = getDate();
	__numFile = 1;
	__nameLogFile = __way + __nameFile + "_(" + __date + ")_" + to_string(__numFile) + ".log";
	__numMassive = 2;
}

void Log::setBufWay(string boof)
{
	__bufWay = boof;
}

void Log::setNameFile(string boof)
{
	__nameFile = boof;
}


void Log::writeLog(int error, string clas, string message)
{
	bool flagFile = checkFile();
	if (flagFile == false)
	{
		ofstream ost(__nameLogFile, ios::app);
		string buffLog = "(" + __date + ")_" + clas + "_" + "\"" + message + "\"";
		if (error != 0)
		{
			buffLog = buffLog + "_" + to_string(error);
		}
		buffLog = buffLog + "\n";
		ost << buffLog;
		ost.close();
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
				__nameLogFile = __way + __nameFile + "_(" + __date + ")_" + to_string(__numFile) + ".log";
			}
			else
			{
				__date = buffDate;
				__numFile = 1;
				__nameLogFile = __way + __nameFile + "_(" + __date + ")_" + to_string(__numFile) + ".log";
			}
		}
		else if (buffDate != __date)
		{
			__date = buffDate;
			__numFile = 1;
			__nameLogFile = __way + __nameFile + "_(" + __date + ")_" + to_string(__numFile) + ".log";
		}
		else
		{
			__nameLogFile = __way + __nameFile + "_(" + __date + ")_" + to_string(__numFile) + ".log";
		}
		ofstream ost(__nameLogFile, ios::app);
		string buffLog = "(" + __date + ")_" + clas + "_" + "\"" + message + "\"";
		if (error != 0)
		{
			buffLog = buffLog + "_" + to_string(error);
		}
		buffLog = buffLog + "\n";
		ost << buffLog;
		ost.close();
	}
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
	ifstream fin(__nameLogFile);
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
	fstream file(__nameLogFile, fstream::in);
	file.seekg(0, ios::end);
	streamsize fileSize = file.tellg();
	file.close();
	return fileSize;

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
