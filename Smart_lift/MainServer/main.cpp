#include <iostream>
#include <csignal>

#include "Modules/SSLSertificateLocalhost/Sertificate.hpp"
#include "Modules/HTTPSServer/HTTPSServer.hpp"
#include <boost/bind.hpp>
#include <boost/lambda2.hpp>
#include "MainServer.hpp"

int main(int argc, char* argv[]) {
    
    // The io_context is required for all I/O
   // HTTPS_Server::net::io_context ioc{ threads };

    // The SSL context is required, and holds certificates
   // HTTPS_Server::ssl::context ctx{ HTTPS_Server::ssl::context::tlsv12 };

    // This holds the self-signed certificate used by the server
   // load_server_certificate(ctx);


    // boost::asio::ip::tcp::endpoint end_point(address, port_MQTT);
    // boost::asio::ip::tcp::acceptor acc(ioc, end_point);

    //TempServer s(ioc, port_MQTT);

    //socket_ptr sock(new boost::asio::ip::tcp::socket(ioc));
    //acc.async_accept(*sock, boost::bind(handle_accept,sock,_1));



    // Create and launch a listening port
    /*std::make_shared<HTTPS_Server::Listener>(
        ioc,
        ctx,
        HTTPS_Server::tcp::endpoint{ address, port },
        doc_root)->run();*/

        // Run the I/O service on the requested number of threads

    setlocale(LC_ALL, "Russian");
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