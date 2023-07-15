#include "Client.hpp"

void failClient(beast::error_code ec, char const* what)
{
	cerr << what << ": " << ec.message() << endl;
}


void Client::RunClient(char const* host, char const* port, char const* target, int version, string body)
{
    req.method(http::verb::post);
    req.target(target);
    req.version(version);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::server, "Beast");
    req.body() = string(body);
    req.set(http::field::content_type, "application/x-www-form-urlencoded");

    //req.set(http::field::body, body);
    auto const size = body.size();
    req.content_length(size);
    req.prepare_payload();
    cout << req << endl;
    resolver.async_resolve(host, port,beast::bind_front_handler(&Client::OnResolve,shared_from_this()));
}

void Client::OnResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
    {
        return failClient(ec, "resolve");
    }
    stream.expires_after(std::chrono::seconds(30));
    stream.async_connect(results,beast::bind_front_handler(&Client::OnConnect,shared_from_this()));
}

void Client::OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec)
    {
        return failClient(ec, "connect");
    }
    stream.expires_after(std::chrono::seconds(30));
    http::async_write(stream, req,beast::bind_front_handler(&Client::OnWrite,shared_from_this()));
}

void Client::OnWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    if (ec)
    {
        return failClient(ec, "write");
    }
    http::async_read(stream, buffer, res,beast::bind_front_handler(&Client::OnRead,shared_from_this()));
}

void Client::OnRead(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        return failClient(ec, "read");
    }

    std::cout << res << std::endl;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected)
    {
        return failClient(ec, "shutdown");
    }
}