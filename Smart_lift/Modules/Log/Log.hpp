#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <locale.h>
#include <ctime>
#include <map>
#include <fstream>

using namespace std;

class Log : public enable_shared_from_this<Log>
{
public:
    Log()
    {
        date = getDate();
        numFile = 1;
        nameFile = "DataBase";
        nameLogFile = way + nameFile + "_(" + date + ")_" + to_string(numFile) + ".log";
        numMassive = 2;
    }
    void writeLog(int error, string clas, string message);
    string nameFile;
    string nameLogFile;
    string date;
    int numFile;
    int numMassive;
    string temporaryLog[3];
    string way = "..\\..\\..\\..\\..\\Smart_lift\\DataBase\\Log\\";
    string getDate();
    bool checkFile();
    streamsize getFileSize();
    void writeTempLog(int error, string clas, string message);
};