#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

beast::string_view MineType(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if (pos == beast::string_view::npos)
        {
            return beast::string_view{};
        }
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"))  return "text/html";
    if (iequals(ext, ".html")) return "text/html";
    if (iequals(ext, ".php"))  return "text/html";
    if (iequals(ext, ".css"))  return "text/css";
    if (iequals(ext, ".txt"))  return "text/plain";
    if (iequals(ext, ".js"))   return "application/javascript";
    if (iequals(ext, ".json")) return "application/json";
    if (iequals(ext, ".xml"))  return "application/xml";
    if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if (iequals(ext, ".flv"))  return "video/x-flv";
    if (iequals(ext, ".png"))  return "image/png";
    if (iequals(ext, ".jpe"))  return "image/jpeg";
    if (iequals(ext, ".jpeg")) return "image/jpeg";
    if (iequals(ext, ".jpg"))  return "image/jpeg";
    if (iequals(ext, ".gif"))  return "image/gif";
    if (iequals(ext, ".bmp"))  return "image/bmp";
    if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff")) return "image/tiff";
    if (iequals(ext, ".tif"))  return "image/tiff";
    if (iequals(ext, ".svg"))  return "image/svg+xml";
    if (iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

string PathCat(beast::string_view base, beast::string_view path);
template <class Body, class Allocator> http::message_generator
HandleReq(beast::string_view root, http::request<Body, http::basic_fields<Allocator>>&& req);

void fail(beast::error_code ErrCode, char const* what);

class HttpSession : public enable_shared_from_this<HttpSession>
{
    beast::tcp_stream stream;
    beast::flat_buffer buffer;
    shared_ptr<string const> root;
    http::request < http::string_body> request;

public:
    HttpSession(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root) : stream(std::move(socket)), root(doc_root)
    {

    }
    void Run();
    void DoRead();
    void OnRead(beast::error_code errorCode, size_t bytesTransferred);
    void SendResponse(http::message_generator&& message);
    void OnWrite(bool keep_alive, beast::error_code errorCode, size_t bytesTransferred);
    void DoClose();
};

class Listener : public enable_shared_from_this<Listener>
{
    net::io_context& ioc;
    tcp::acceptor acceptor;
    shared_ptr<string const> const& doocRoot;

public:
    Listener(net::io_context& Ioc, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root)
        : ioc(Ioc)
        , acceptor(net::make_strand(Ioc))
        , doocRoot(doc_root)
    {
        beast::error_code errorCode;
        acceptor.open(endpoint.protocol(), errorCode);
        if (errorCode)
        {
             fail(errorCode, "open");
             return;
        }

        acceptor.set_option(net::socket_base::reuse_address(true), errorCode);
        if (errorCode)
        {
            fail(errorCode, "set_option");
            return;
        }
        
        acceptor.bind(endpoint, errorCode);
        if (errorCode)
        {
            fail(errorCode, "bind");
            return;
        }

        acceptor.listen(net::socket_base::max_listen_connections, errorCode);

        if (errorCode)
        {
            fail(errorCode, "listen");
            return;
        }
    }
    void Run();

private:
    void __DoAccept();
    void __OnAccept(beast::error_code errorCode, tcp::socket socket);
};

