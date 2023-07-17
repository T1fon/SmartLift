/*#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <locale.h>
#include <ctime>
#include <map>
#include <fstream>
#include "../Log/Log.hpp"

using namespace std;

#define CONFIG_FILE "config.txt"
#define NOT_FOUND_CONFIG_FILE 1


class Config : public enable_shared_from_this<Config>
{
public:
    Config(shared_ptr<Log> lg, string boofWay);
    map<string, string>getConfigInfo();
    void writeError(int error);
    void readConfig();
    void setLog(shared_ptr<Log> lg);
    void setWay(string boof);
private:
    string __conf = CONFIG_FILE;
    string __boofWay;
    string __way = "..\\..\\..\\..\\..\\Smart_lift\\";
    shared_ptr<Log> __logConfig;
    map<string, string> __configInfo;

};*/