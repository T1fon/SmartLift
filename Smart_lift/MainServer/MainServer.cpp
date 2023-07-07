#include <iostream>
#include <csignal>
//#include <libs/beast/example/common/server_certificate.hpp>
#include "Modules/SSLSertificateLocalhost/Sertificate.hpp"
#include "Modules/HTTPSServer/HTTPSServer.hpp"
#include <boost/bind.hpp>
#include <boost/lambda2.hpp>
//#include <Modules/HTTPSServer/HTTPSServer.hpp>
//#include <Modules/MQTTBroker/MQTTBroker.hpp>

using namespace std;
typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

class TempSession: public std::enable_shared_from_this<TempSession> {
private:
    boost::asio::ip::tcp::socket __socket;
    static const size_t __MAX_LENGTH = 1024;
    unsigned char __data[__MAX_LENGTH];
    
    void __doRead(){
        auto self(shared_from_this());
        __socket.async_read_some(boost::asio::buffer(__data, __MAX_LENGTH-1),
            [this, self](boost::system::error_code err_code, size_t length) {
                if (!err_code) {
                    try {
                        cout << "Data: ";
                        for (int i = 0; i < __MAX_LENGTH; i++) {
                            if ((int)__data[i] == 205) {
                                break;
                            }
                            cout << (int)__data[i] << " ";
                        }
                        cout << endl;
                    }
                    catch (std::exception& e) {
                        cout << "Exc " << e.what() << endl;
                    }
                    
                    
                    //return __doRead();
                    __doRead();
                }
                else {
                    //cout << "Error" << err_code.message();
                    __doRead();
                }
            }
        );
    }
    void __doWrite(std::size_t length){}
public:
    TempSession(boost::asio::ip::tcp::socket socket) :__socket(std::move(socket)) {

    }
    void start() {
        __doRead();
    }
};

class TempServer {
private:
    boost::asio::ip::tcp::acceptor __acceptor;
    std::shared_ptr<TempSession> __session;
    void __doAccept() {
        __acceptor.async_accept([this](boost::system::error_code err_code, boost::asio::ip::tcp::socket socket)
            {
                if (err_code) {
                    //мб ошибка
                    //return __doAccept();
                    __doAccept();
                }
                std::make_shared<TempSession>(std::move(socket))->start();
            }
        );
    }
public:
    TempServer(boost::asio::io_context& io_context, short port):__acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {
        __doAccept();
    }
};

int main(int argc, char* argv[]) {
    // Check command line arguments.ip
    /*if (argc != 5)
    {
        std::cerr <<
            "Usage: http-server-async-ssl <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    http-server-async-ssl 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }*/
    //auto const address = HTTPS_Server::net::ip::make_address(argv[1]);
    //auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    //auto const doc_root = std::make_shared<std::string>(argv[3]);
    //auto const threads = std::max<int>(1, std::atoi(argv[4]));

    auto const address = HTTPS_Server::net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("443"));
    auto const port_MQTT = static_cast<unsigned short>(std::atoi("1883"));
    auto const doc_root = std::make_shared<std::string>("/");
    auto const threads = std::max<int>(1, std::atoi("3"));

    // The io_context is required for all I/O
    HTTPS_Server::net::io_context ioc{ threads };

    // The SSL context is required, and holds certificates
    HTTPS_Server::ssl::context ctx{ HTTPS_Server::ssl::context::tlsv12 };

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);


   // boost::asio::ip::tcp::endpoint end_point(address, port_MQTT);
   // boost::asio::ip::tcp::acceptor acc(ioc, end_point);
    
    TempServer s(ioc, port_MQTT);

    //socket_ptr sock(new boost::asio::ip::tcp::socket(ioc));
    //acc.async_accept(*sock, boost::bind(handle_accept,sock,_1));
    


    // Create and launch a listening port
    /*std::make_shared<HTTPS_Server::Listener>(
        ioc,
        ctx,
        HTTPS_Server::tcp::endpoint{ address, port },
        doc_root)->run();*/

    // Run the I/O service on the requested number of threads




    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    //cout << "bebebe" << endl;

    return EXIT_SUCCESS;
}