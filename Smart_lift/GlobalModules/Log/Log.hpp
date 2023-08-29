#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <locale.h>
#include <ctime>
#include <map>
#include <memory>
#include <fstream>

using namespace std;


class Log : public enable_shared_from_this<Log>
{
public:
    Log(string way, string root_directory, string name_class);
    void writeTempLog(int error, string clas, string message);
    void writeLog(int error, string clas, string message);
    void setFinalPath();
    streamsize getFileSize();
    string getDate();
private:
    bool __checkFile();
    void __writeLogToFile(string clas, string message, int error);
    string __name_file;
    string __final_path;
    string __date;
    int __num_file;
    int __num_massive;
    string __temporary_log[3];
    string __way;
    string __buf_way;
    string __root_directory;

};

