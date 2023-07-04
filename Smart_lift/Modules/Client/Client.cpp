#include "Client.hpp"

void failClient(beast::error_code ec, char const* what)
{
	cerr << what << ": " << ec.message() << endl;
}

void Client::RunClient(char const* host, char const* port, char const* target, int version)
{

    req.version(version);
    req.method(http::verb::get);
    req.target(target);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    resolver.async_resolve(host, port,beast::bind_front_handler(&Client::OnResolve,shared_from_this()));
    cout << 1 << endl;
}

void Client::OnResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
    {
        return failClient(ec, "resolve");
    }
    stream.expires_after(std::chrono::seconds(30));
    stream.async_connect(results,beast::bind_front_handler(&Client::OnConnect,shared_from_this()));
    cout << 2 << endl;
}

void Client::OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec)
    {
        return failClient(ec, "connect");
    }
    stream.expires_after(std::chrono::seconds(30));
    http::async_write(stream, req,beast::bind_front_handler(&Client::OnWrite,shared_from_this()));
    cout << 3 << endl;
}

void Client::OnWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    if (ec)
    {
        return failClient(ec, "write");
    }
    cout << 4.1 << endl;
    http::async_read(stream, buffer, res,beast::bind_front_handler(&Client::OnRead,shared_from_this()));
    cout << 4 << endl;
}

void Client::OnRead(beast::error_code ec, std::size_t bytes_transferred)
{
    cout << 5.1 << endl;
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        return failClient(ec, "read");
    }
    cout << 5.2 << endl;

    std::cout << res << std::endl;
    cout << 5.3 << endl;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected)
    {
        return failClient(ec, "shutdown");
    }
    cout << 5 << endl;
}