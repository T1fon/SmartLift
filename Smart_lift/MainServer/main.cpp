#include <iostream>

#include "Modules/SSLSertificateLocalhost/Sertificate.hpp"
#include "Modules/HTTPSServer/HTTPSServer.hpp"
#include <boost/bind.hpp>
#include <boost/lambda2.hpp>
#include "MainServer.hpp"

int main(int argc, char* argv[]) {
    
  
        // Run the I/O service on the requested number of threads

    setlocale(LC_ALL, ".UTF-8");
    string config_file_name = "";
    for (int i = 1; i < argc; i++) {
        string flags = argv[i];
        if (flags == "-cf" || flags == "--config_file") {
            config_file_name = argv[++i];
        }
    }

    MainServer ms;
    if (ms.init(config_file_name) != MainServer::PROCESS_CODE::SUCCESSFUL) {
        return -1;
    }
    ms.start();


    return EXIT_SUCCESS;
}