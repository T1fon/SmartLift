#include "Server.hpp"

HttpConnection::HttpConnection(tcp::socket socket) : __socket(move(socket))
{

}

void HttpConnection::Start()
{
	__ReadRequest();
	__CheckDeadline();
}

void HttpConnection::__ReadRequest()
{
	auto self = shared_from_this();

	http::async_read(
		__socket,
		__buffer,
		__request,
		[self](beast::error_code ErrorCode, size_t BytesTransferred)
		{
			boost::ignore_unused(BytesTransferred);
			if (!ErrorCode)
			{
				self->__ProcessRequest();
			}
		}
	);
}

void HttpConnection::__ProcessRequest()
{
	__response.version(__request.version());
	__response.keep_alive(false);

	switch (__request.method())
	{
	case http::verb::get:
		__response.result(http::status::ok);
		__response.set(http::field::server, "Beast");
		__CreateResponse();
	default:
		__response.result(http::status::bad_request);
		__response.set(http::field::content_type, "text/plain");
		beast::ostream(__response.body()) << "Invalid request-method ' " << string(__request.method_string()) << "'";
		break;
	}
	__WriteResponse();
}

void HttpConnection::__CreateResponse()
{
	///Послать ответ на ЛБ
}

void HttpConnection::__WriteResponse()
{
	auto self = shared_from_this();

	__response.content_length(__response.body().size());

	http::async_write(
		__socket,
		__response,
		[self](beast::error_code ErrorCode, size_t)
		{
			self->__socket.shutdown(tcp::socket::shutdown_send, ErrorCode);
			self->__deadline.cancel();
		}
	);
}

void HttpConnection::__CheckDeadline()
{
	auto self = shared_from_this();

	__deadline.async_wait(
		[self](beast::error_code ErrorCode)
		{
			if (!ErrorCode)
			{
				self->__socket.close(ErrorCode);
			}
		}
	);
}

void HttpServer(tcp::acceptor& acceptor, tcp::socket& socket)
{
	cout << "Sever is Working" << endl;
	acceptor.async_accept(socket,
		[&](beast::error_code ErrorCode)
		{
			if (!ErrorCode)
			{
				make_shared<HttpConnection>(move(socket))->Start();
			}
			HttpServer(acceptor, socket);
		});
}
	

