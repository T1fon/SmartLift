#include "Server.hpp"


beast::string_view
mime_type(beast::string_view path)
{
    {
        using beast::iequals;
        auto const ext = [&path]
        {
            auto const pos = path.rfind(".");
            if (pos == beast::string_view::npos)
                return beast::string_view{};
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
}

string PathCat(beast::string_view base, beast::string_view path)
{
	if (base.empty())
	{
		return string(path);
	}
	string result(base);
#ifdef BOOST_MSVC
	char constexpr pathSepp = '\\';
	if (result.back() == pathSepp)
	{
		result.resize(result.size() - 1);
	}
	result.append(path.data(), path.size());
	for (auto& c : result)
	{
		if (c == '/')
		{
			c = pathSepp;
		}
	}
#else
	char constexpr path_separator = '/';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
#endif
	return result;
}

template <class Body, class Allocator>
http::message_generator
HandleReq(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    auto const bad_request =
        [&req](beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    auto const not_found =
        [&req](beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    auto const server_error =
        [&req](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    if (req.method() != http::verb::get &&
        req.method() != http::verb::head)
    {
        return bad_request("Unknown HTTP-method");
    }

    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
    {
        return bad_request("Illegal request-target");
    }

    std::string path = PathCat(doc_root, req.target());
    if (req.target().back() == '/')
    {
        path.append("index.html");
    }

    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    if (ec == beast::errc::no_such_file_or_directory)
    {
        return not_found(req.target());
    }

    if (ec)
    {
        return server_error(ec.message());
    }

    auto const size = body.size();

    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    http::response<http::file_body> res{
        std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;
}

void fail(beast::error_code ErrCode, char const* what)
{
	cerr << what << ": " << ErrCode.message() << endl;
}

void HttpSession::Run()
{
	net::dispatch(stream.get_executor(), beast::bind_front_handler(&HttpSession::DoRead, shared_from_this()));
    cout << 14 << endl;
}

void HttpSession::DoRead()
{
	request = {};
	stream.expires_after(chrono::seconds(30));
	http::async_read(stream, buffer, request, beast::bind_front_handler(&HttpSession::OnRead, shared_from_this()));
    cout << 15 << endl;
}

void HttpSession::OnRead(beast::error_code errorCode, size_t bytesTransferred)
{
	boost::ignore_unused(bytesTransferred);
	if (errorCode == http::error::end_of_stream)
	{
        cout << 16.1 << endl;
		return DoClose();
	}
	if (errorCode)
	{
        cout << 16.2 << endl;
		return fail(errorCode, "readServer");
	}
    cout << 16.3 << endl;
	SendResponse(HandleReq(*root, std::move(request)));
}

void HttpSession::SendResponse(http::message_generator&& message)
{
	bool keepAlive = message.keep_alive();
	beast::async_write(stream, move(message), beast::bind_front_handler(&HttpSession::OnWrite, shared_from_this(), keepAlive));
    cout << 17 << endl;
}

void HttpSession::OnWrite(bool keep_alive, beast::error_code errorCode, size_t bytesTransferred)
{
	if (errorCode)
	{
        cout << 18.1 << endl;
		return fail(errorCode, "write");
	}
	if (!keep_alive)
	{
        cout << 18.2 << endl;
		return DoClose();
	}
    cout << 18.3 << endl;
	DoRead();
}

void HttpSession::DoClose()
{
	beast::error_code errorCode;
	stream.socket().shutdown(tcp::socket::shutdown_send, errorCode);
    cout << 19 << endl;
}

void Listener::Run()
{
	__DoAccept();
    cout << 11 << endl;
}

void Listener::__DoAccept()
{
	acceptor.async_accept(net::make_strand(ioc), beast::bind_front_handler(&Listener::__OnAccept, shared_from_this()));
    cout << 12 << endl;
}

void Listener::__OnAccept(beast::error_code errorCode, tcp::socket socket)
{
	if (errorCode)
	{
		fail(errorCode, "accept");
        cout << 13.1 << endl;
		return;
	}
	else
	{
		make_shared<HttpSession>(move(socket), doocRoot)->Run();
        cout << 13.2 << endl;
	}
	__DoAccept();
}