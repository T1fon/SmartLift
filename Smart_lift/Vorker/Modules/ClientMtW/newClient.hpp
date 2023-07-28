#include <boost/lambda2.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <locale.h>
#include <ctime>
#include <map>
#include <list>
#include <queue>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;


class Client : public enable_shared_from_this<Client>
{
public:
	Client(string ip, string port, net::io_context& ioc, queue<string> message);//добавить очередь из запросов
	~Client();
	void start();
	void stop();

	void __reqConnect(const boost::system::error_code& eC);
	void __sendConnect(const boost::system::error_code& eC, size_t bytesSend);
	void __reciveCommand(const boost::system::error_code& eC, size_t bytesRecive);
	void __commandAnalize();


private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	shared_ptr<tcp::endpoint> __endPoint;
	shared_ptr<tcp::socket> __socket;
	static const int BUF_RECIVE_SIZE = 2048;
	queue<string> __bufSend;
	string __bufQueueString;
	char* __bufRecive;
	boost::json::value __bufJsonRecive;
	boost::json::stream_parser __parser;

	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __checkSend(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);
	__CHECK_STATUS __checkJson(const size_t& countReciveByte, __handler_t&& handler);

};