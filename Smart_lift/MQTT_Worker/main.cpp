#include "MQTTWorker.hpp"

#include <iostream>
using namespace std;
int main(int argc, char** argv) {


    
    MQTTWorker worker;
    worker.init();
    worker.start();
    return 0;

}
