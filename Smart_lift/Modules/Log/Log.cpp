#include "Log.hpp"


void Log::writeLog(int error, string clas, string message)
{
	bool flagFile = checkFile();
	if (flagFile == false)
	{
		ofstream ost(nameLogFile, ios::app);
		string buffLog = "(" + date + ")_" + clas + "_" + "\"" + message + "\"";
		if (error != 0)
		{
			buffLog = buffLog + "_" + to_string(error);
		}
		buffLog = buffLog + "\n";
		ost << buffLog;
		ost.close();
		cout << 0 << endl;
	}
	else
	{
		string buffDate = getDate();
		streamsize fileSize = getFileSize();
		if (fileSize >= 204800)
		{
			if (date == buffDate)
			{
				numFile++;
				nameLogFile = way + nameFile + "_(" + date + ")_" + to_string(numFile) + ".log";
				cout << 1 << endl;
			}
			else
			{
				date = buffDate;
				numFile = 1;
				nameLogFile = way + nameFile + "_(" + date + ")_" + to_string(numFile) + ".log";
				cout << 2 << endl;
			}
		}
		else if (buffDate != date)
		{
			date = buffDate;
			numFile = 1;
			nameLogFile = way + nameFile + "_(" + date + ")_" + to_string(numFile) + ".log";
			cout << 4 << endl;
		}
		else
		{
			nameLogFile = way + nameFile + "_(" + date + ")_" + to_string(numFile) + ".log";
			cout << 5 << endl;
		}
		ofstream ost(nameLogFile, ios::app);
		string buffLog = "(" + date + ")_" + clas + "_" + "\"" + message + "\"";
		if (error != 0)
		{
			buffLog = buffLog + "_" + to_string(error);
		}
		buffLog = buffLog + "\n";
		ost << buffLog;
		ost.close();
		cout << 6 << endl;
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
	ifstream fin(nameLogFile);
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
	fstream file(nameLogFile, fstream::in);
	file.seekg(0, ios::end);
	streamsize fileSize = file.tellg();
	file.close();
	return fileSize;

}

void Log::writeTempLog(int error, string clas, string message)
{
	string buffLog = "(" + date + ")_" + clas + "_" + "\"" + message + "\"";
	if (error != 0)
	{
		buffLog = buffLog + "_" + to_string(error);
	}
	switch (numMassive)
	{
	case 2:
	{
		temporaryLog[numMassive] = buffLog;
		numMassive = 1;
		break;
	}
	case 1:
	{
		temporaryLog[numMassive] = temporaryLog[numMassive + 1];
		temporaryLog[numMassive + 1] = buffLog;
		numMassive = 0;
		break;
	}
	case 0:
	{
		for (int i = 0; i < 2; i++)
		{
			temporaryLog[i] = temporaryLog[i + 1];
		}
		numMassive = 2;
		temporaryLog[numMassive] = buffLog;
		break;
	}
	default:
		break;
	}
}