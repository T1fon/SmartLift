#pragma once
#include <algorithm>
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
    Config(shared_ptr<Log> lg, string root_directory, string way, string file_name = CONFIG_FILE);
    
    map<string, string>getConfigInfo();    
    void readConfig();
    void setLog(shared_ptr<Log> lg);
    void setWay(string way);
    void setRootDirectory(string root_directory);
    void setFileName(string file_name);
private:
    
    string __root_directory;
    string __way;
    string __file_name;
    string __final_path;
    shared_ptr<Log> __log_config;
    map<string, string> __config_info;
    void __writeError(int error);
};