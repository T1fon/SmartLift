#include "MQTTWorker.hpp"
#include <string>
#include <iostream>
using namespace std;
int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");
    string config_file_name = "";
    for (int i = 1; i < argc; i++) {
        string flags = argv[i];
        if (flags == "-cf" || flags == "--config_file") {
            config_file_name = argv[++i];
        }
    }

    
    MQTTWorker worker(config_file_name);
    if (worker.init() != MQTTWorker::SUCCESSFUL) {
        cout << "Error reading Config file" << endl;
        return -1;
    }
    worker.start();
    return 0;

}
