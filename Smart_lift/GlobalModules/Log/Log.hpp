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
    Log(string bufWay, string name);
    void writeTempLog(int error, string clas, string message);
    void writeLog(int error, string clas, string message);
    string getDate();
    bool checkFile();
    streamsize getFileSize();
    void setBufWay(string boof);
    void setNameFile(string boof);
private:
    string __nameFile;
    string __nameLogFile;
    string __date;
    int __numFile;
    int __numMassive;
    string __temporaryLog[3];
    string __way = "..\\..\\..\\..\\..\\Smart_lift\\";
    string __bufWay;
};

