#include "Vorker.hpp"


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

    /*auto const not_found =
        [&req](beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };*/

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
        req.method() != http::verb::head && 
        req.method() != http::verb::post)
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

    /*if (ec == beast::errc::no_such_file_or_directory)
    {
        return not_found(req.target());
    }

    if (ec)
    {
        return server_error(ec.message());
    }*/
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
    else if(req.method() == http::verb::get)
    {
        http::response<http::file_body> res{
            std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        res.body().open("file.jpeg", boost::beast::file_mode::write, ec);
        body.close();
        return res;
    }
    else if (req.method() == http::verb::post)
    {
        http::response<http::string_body> res;
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        string boofer = req.body();
        string secBoof = Json(boofer);
        res.body() = string(secBoof);
        res.content_length(secBoof.size());
        res.keep_alive(req.keep_alive());
        return res;
    }
}

void fail(beast::error_code ErrCode, char const* what)
{
	cerr << what << ": " << ErrCode.message() << endl;
}

void HttpSession::Run()
{
    cout << 1 << endl;
	net::dispatch(stream.get_executor(), beast::bind_front_handler(&HttpSession::DoRead, shared_from_this()));
}

void HttpSession::DoRead()
{
    cout << 2 << endl;
	request = {};
	stream.expires_after(chrono::seconds(30));
	http::async_read(stream, buffer, request, beast::bind_front_handler(&HttpSession::OnRead, shared_from_this()));
}

void HttpSession::OnRead(beast::error_code errorCode, size_t bytesTransferred)
{
	boost::ignore_unused(bytesTransferred);
	if (errorCode == http::error::end_of_stream)
	{
		return DoClose();
	}
	if (errorCode)
	{
        cout << request << endl;
		return fail(errorCode, "readServer");
	}
    //chooseResponce();
	SendResponse(HandleReq(*root, std::move(request)));
}

void HttpSession::chooseResponce()
{
    readJson();
    string target = __root.get<string>("target");
    if(target == "connect")

    {
        bool flag = checkConnect();
        if (flag)
        {
            //цспешный коннект
        }
        else
        {
            //ошибка нет коннекта
        }
    }
    else if (target == "DB_query")
    {
        string method = __root.get<string>("method");
        string fields = __root.get<string>("field");
        __query.clear();
        __query = __root.get<string>("query");
        int boof = sqlite3_exec(__dB, __query.c_str(), HttpSession::callback, NULL, NULL);
        if (boof == 0)
        {
            //ошибка нет данных
        }
        else
        {
            if (method == "select")
            {
                //c
            }
        }
    }
    else if (target == "disconnect")
    {

    }
}

bool HttpSession::checkConnect()
{
    string login = __root.get<string>("login");
    string password = __root.get<string>("password");
    int exit = sqlite3_open(DB_FILE_WAY, &__dB);
    __query.clear();
    __query = "SELECT FROM Accounts WHERE Login = " + login;
    int boof = sqlite3_exec(__dB, __query.c_str(),HttpSession::callback, NULL, NULL);
    if (boof == 0)
    {
        return false;
    }
    else
    {
        if (password == __reqRes)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

int HttpSession::callback(void* NotUsed, int argc, char** argv, char** azColName)
{
    if (argc == 0)
    {
        return 0;
    }
    __reqRes.clear();
    __reqRes = argv[0];
    return 1;
}

void HttpSession::readJson()
{
    string body = request.body();
    stringstream encoded(body);
    boost::property_tree::read_json(encoded, __root);
}

void HttpSession::SendResponse(http::message_generator&& message)
{
	bool keepAlive = message.keep_alive();
	beast::async_write(stream, move(message), beast::bind_front_handler(&HttpSession::OnWrite, shared_from_this(), keepAlive));
}

void HttpSession::OnWrite(bool keep_alive, beast::error_code errorCode, size_t bytesTransferred)
{
	if (errorCode)
	{
		return fail(errorCode, "write");
	}
	if (!keep_alive)
	{
		return DoClose();
	}
	DoRead();
}

void HttpSession::DoClose()
{
	beast::error_code errorCode;
	stream.socket().shutdown(tcp::socket::shutdown_send, errorCode);
}

void Listener::Run()
{
	__DoAccept();
}

void Listener::__DoAccept()
{
	acceptor.async_accept(net::make_strand(ioc), beast::bind_front_handler(&Listener::__OnAccept, shared_from_this()));
}

void Listener::__OnAccept(beast::error_code errorCode, tcp::socket socket)
{
	if (errorCode)
	{
		fail(errorCode, "accept");
		return;
	}
	else
	{
		make_shared<HttpSession>(move(socket), doocRoot)->Run();
	}
	__DoAccept();
}

string Json(string body)
{
    stringstream encoded(body);
    boost::property_tree::ptree root;
    boost::property_tree::read_json(encoded, root);
    string boofer = root.get<string>("text");
    string t = "fuck";
    if (boofer == "hellow")
    {
        root.put("text", t);
    }
    else
    {
        root.put("text", " ");
    }
    boost::property_tree::write_json(encoded, root);

    return encoded.str();
}



int main(int argc, char* argv[])
{
    try
    {

        string add = "0.0.0.0";
        string p = "80";
        const char* addr = "127.0.0.1";
        const char* por = "80";
        string doocRoot = ".";
        const char* target = "/";
        auto const address = net::ip::make_address(add);
        auto const port = static_cast<unsigned short>(atoi("80"));
        auto const docRoot = make_shared<string>(doocRoot);
        auto const threads = max<int>(1, 2);

        boost::property_tree::ptree doc;
        doc.put("Id", "145415fd515454f");
        doc.put("text", "hellow");
        stringstream oss;
        boost::property_tree::write_json(oss, doc);
        string mainbody = oss.str();
        //cout << mainbody << endl;

        net::io_context ioc{threads};
        make_shared<Listener>(ioc, tcp::endpoint{address, port}, docRoot)->Run();
        vector<thread> v;
        v.reserve(threads - 1);
        for (auto i = threads - 1; i > 0; i--)
        {
            v.emplace_back([&ioc]
                {
                    ioc.run();
                }
            );
        }
        make_shared<Client>(ioc)->RunClient(addr, por, target, 11, mainbody);
        ioc.run();
    }
    catch (exception const& e)
    {
        cerr << "Error" << e.what() << endl;
        return EXIT_FAILURE;
    }
}
