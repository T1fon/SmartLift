#include <iostream>
#include <csignal>

#include "Modules/SSLSertificateLocalhost/Sertificate.hpp"
#include "Modules/HTTPSServer/HTTPSServer.hpp"
#include <boost/bind.hpp>
#include <boost/lambda2.hpp>
#include "MainServer.hpp"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    auto const address = HTTPS_Server::net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("443"));
    auto const port_MQTT = static_cast<unsigned short>(std::atoi("1883"));
    auto const doc_root = std::make_shared<std::string>("/");
    auto const threads = std::max<int>(1, std::atoi("3"));

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


    MainServer ms({});
    ms.init();
    ms.start();

    /*std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    //cout << "bebebe" << endl;*/

    return EXIT_SUCCESS;
}