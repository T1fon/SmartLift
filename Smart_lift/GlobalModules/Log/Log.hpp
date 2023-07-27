#pragma once
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
    Log(string way, string rootDirectory, string nameClass);
    void writeTempLog(int error, string clas, string message);
    void writeLog(int error, string clas, string message);
    string getDate();
    bool checkFile();
    streamsize getFileSize();
    void setFinalPath();
    void writeLogToFile(string clas, string message, int error);
private:
    string __nameFile;
    string __finalPath;
    string __date;
    int __numFile;
    int __numMassive;
    string __temporaryLog[3];
    string __way;
    string __bufWay;
    string __rootDirectory;

};
