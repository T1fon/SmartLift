#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = boost::asio::ip::tcp;
using namespace std;

/*namespace my_program_state
{
	size_t request_count()
	{
		static size_t CountRequest = 0;
			return ++CountRequest;
	}

	time_t Now()
	{
		return time(0);
	}
}*/

class HttpConnection : public enable_shared_from_this<HttpConnection>
{
public:
	HttpConnection(tcp::socket socket);

	void Start();

private:
	tcp::socket __socket;
	beast::flat_buffer __buffer{8192};
	http::request<http::dynamic_body> __request;
	http::response<http::dynamic_body> __response;
	net::steady_timer __deadline
	{
		__socket.get_executor(),
			chrono::seconds(60)
	};

	void __ReadRequest();
	void __ProcessRequest();
	void __CreateResponse();
	void __WriteResponse();
	void __CheckDeadline();

};
void HttpServer(tcp::acceptor& acceptor, tcp::socket& socket);
