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
    Config(Log& lg) : logConfig(lg)
    {
        ifstream fin(way);
        if (!fin.is_open())
        {
            writeError(NOT_FOUND_CONFIG_FILE);
        }
        else
        {
            fin.close();
        }

    }
    string conf = CONFIG_FILE;
    string way = "..\\..\\..\\..\\..\\Smart_lift\\DataBase\\" + conf;
    Log logConfig;
    void writeError(int error);
    map<string, string> readConfig();

};