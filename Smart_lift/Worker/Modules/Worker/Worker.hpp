#pragma once
#include "../../../GlobalModules/Log/Log.hpp"
#include "../../../GlobalModules/Config/Config.hpp"
#include "../../../GlobalModules/JSONFormatter/JSONFormatter.hpp"
#include "../../../GlobalModules/ClientDB/ClientDB.hpp"
#include <boost/lambda2.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast.hpp> 
#include <boost/locale/encoding.hpp>
#include <boost/locale/generator.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <locale.h>
#include <ctime>
#include <map>
#include <list>
#include <queue>
#include <iostream>

using namespace std;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;


class Worker : public enable_shared_from_this<Worker>
{
public:
	typedef std::function<void(map<string, map<string, vector<string>>> )> _callback_t;
	_callback_t callback;

	void __сallback(map<string, map<string, vector<string>>> data)
	{
		setDbInfo(data);
	}

	Worker(map<string, string> conf_info, net::io_context& ioc, shared_ptr<Log> lg) :__ioc(ioc),
		callback(boost::bind(&Worker::__сallback, this, boost::placeholders::_1))
	{
		cout << 1 << endl;
		__socket = make_shared<tcp::socket>(__ioc);
		__buf_recive = new char[BUF_RECIVE_SIZE + 1];
		__buf_send = "";
		__buf_json_recive = {};
		__parser.reset();
		__config_info = conf_info;
		__timer = make_shared<net::deadline_timer>(__ioc);
		__log = lg;
		__flag_disconnect = false;

	}
	~Worker()
	{
		this->stop();
		delete[] __buf_recive;
	}
	void start()
	{
		cout << 2 << endl;
		__log->writeLog(0, __name, "start_server");
		__log->writeTempLog(0, __name, "start_server");
		__timer->expires_from_now(boost::posix_time::hours(24));
		__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
		try
		{
			__ip_ms = __config_info.at("Main_server_ip");
			__port_ms = __config_info.at("Main_server_port");
			__worker_id = __config_info.at("Id");
			__ip_db = __config_info.at("BD_ip");
			__port_db = __config_info.at("BD_port");
			__db_log = __config_info.at("BD_login");
			__db_pas = __config_info.at("BD_password");
			__socket_dB = make_shared<tcp::socket>(__ioc);
			__db_client = make_shared<ClientDB>(__ip_db, __port_db, __db_log, __db_pas, __name, __socket_dB, callback);
			__end_point = make_shared<tcp::endpoint>(tcp::endpoint(net::ip::address::from_string(__ip_ms), stoi(__port_ms)));


		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			this->stop();
			return;
		}
		__connectToBd();
		__connectToMS();
	}
	void stop()
	{
		if (__socket->is_open())
		{
			__socket->close();
		}
	}

	void setDbInfo(map <string, map<string, vector<string>>> data)
	{
		__db_info = data;

	}

private:
	typedef std::function<void(boost::system::error_code, std::size_t)> __handler_t;
	string __name = "Marusia_Worker";
	shared_ptr<tcp::socket>__socket;
	shared_ptr<tcp::socket> __socket_dB;
	net::io_context& __ioc;
	shared_ptr<ClientDB> __db_client;
	shared_ptr<net::deadline_timer> __timer;

	string __id;
	string __ip_ms;
	string __port_ms;
	string __worker_id;
	string __ip_db;
	string __port_db;
	string __db_log;
	string __db_pas;

	map<string, map<string, vector<string>>> __db_info;
	/*vector<string> __marussiaStationFields = {"ApplicationId", "ComplexId", "HouseNum"};
	vector<string> __houseFields = { "TopFloor", "BottomFloor", "NullFloor", "HouseNum", "ComplexId" };
	vector<string> __staticPhrasesFields = { "ComplexId", "HouseNumber", "KeyWords", "Response" };*/


	static const int BUF_RECIVE_SIZE = 2048;
	string __buf_send;
	char* __buf_recive;
	queue<boost::json::value> __buf_json_recive;
	boost::json::value __buf_json_response;
	boost::json::stream_parser __parser;
	queue<string> __sender;
	shared_ptr<Log> __log;
	shared_ptr<tcp::endpoint> __end_point;
	bool __flag_disconnect = false;
	bool __flag_end_send = false;
	string __response_command;
	boost::json::value __json_string;

	vector<u8string> __key_roots = { u8"перв", u8"втор", u8"трет", u8"чет", u8"пят", u8"шест", u8"седьм", u8"восьм", u8"девят",u8"дцат", u8"сорок", u8"десят",  u8"девян", u8"сот",u8"сто",u8"минус", u8"нулев"};
	map<u8string, int> __num_roots= {
		{u8"минус третий", -3}, {u8"минус второй", -2}, {u8"минус первый", -1}, {u8"нулевой", 0},
		{u8"первый", 1}, {u8"второй", 2}, {u8"третий", 3}, {u8"четвертый", 4}, {u8"пятый", 5}, {u8"шестой", 6}, {u8"седьмой", 7}, {u8"восьмой", 8},{u8"девятый", 9}, {u8"десятый", 10},
		{u8"одиннадцатый", 11}, {u8"двенадцатый", 12}, {u8"тринадцатый", 13}, {u8"четырнадцатый", 14}, {u8"пятнадцатый", 15}, {u8"шестнадцатый", 16}, {u8"семнадцатый", 17}, {u8"восемнадцатый", 18},
		{u8"девятнадцатый", 19}, {u8"двадцатый", 20}, {u8"двадцать первый", 21}, {u8"двадцать второй", 22}, {u8"двадцать третий", 23}, {u8"двадцать четвертый", 24}, {u8"двадцать пятый", 25},
		{u8"двадцать шестой", 26}, {u8"двадцать седьмой", 27}, {u8"двадцать восьмой", 28}, {u8"двадцать девятый", 29}, {u8"тридцатый", 30}
	};
	vector<u8string> __mqtt_keys = { u8"этаж",u8"подъем",u8"спуск",u8"подними",u8"опусти" };

	map<string, string> __config_info;

	void __connectToMS()
	{
		__socket->async_connect(*__end_point, boost::bind(&Worker::__sendConnect, shared_from_this(), boost::placeholders::_1));
	}

	void __sendConnect(const boost::system::error_code& eC)
	{
		if (eC)
		{
			__log->writeLog(3, __name, "Error_failed_to_connect");
			__log->writeTempLog(3, __name, "Error_failed_to_connect");
			cerr << "connect" << eC.message() << endl;
			Sleep(2000);
			this->stop();
			this->start();
			return;
		}

		__buf_send.clear();
		__buf_send = boost::json::serialize(json_formatter::worker::request::connect(__name, __worker_id));
		__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__reciveConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void __reciveConnect(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			cerr << eC.message() << endl;
			cerr << "reciveConnect" << endl;
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__reciveConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		static size_t temp_send = 0;
		temp_send += bytes_send;
		if (temp_send != __buf_send.size())
		{
			__log->writeTempLog(0, "DataBase", "Not_full_json");
			__socket->async_send(net::buffer(__buf_send.c_str() + temp_send, __buf_send.size() - temp_send), boost::bind(&Worker::__reciveConnect, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_send = 0;
		//cerr << __buf_send << endl;
		__buf_send.clear();
		__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void __reciveCommand(const boost::system::error_code& eC, size_t bytes_recive)
	{
		if (eC)
		{
			cerr << "reciveCommand " << eC.message() << endl;
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		size_t count_byte, count_byte_write = 0;
		for (; count_byte_write < bytes_recive;)
		{
			try
			{
				count_byte = __parser.write_some(__buf_recive + count_byte_write);
			}
			catch (exception& e)
			{
				cerr << e.what() << endl;
			}
			if (!__parser.done())
			{
				fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
				cerr << "Json is not full" << endl;
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
				return;
			}
			try
			{
				__buf_json_recive.push(__parser.release());
				__parser.reset();
				count_byte_write += count_byte;
			}
			catch (exception& e)
			{
				__parser.reset();
				__buf_json_recive = {};
				cerr << "Json read " << e.what() << endl;
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
				return;
			}
		}
		fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
		__buf_json_recive = {};
		__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__sendResponse, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void __sendResponse(const boost::system::error_code& eC, size_t bytes_recive)
	{
		if (eC)
		{
			cerr << "reciveCommand " << eC.message() << endl;
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}

		size_t count_byte, count_byte_write = 0;
		for (; count_byte_write < bytes_recive;)
		{
			try
			{
				count_byte = __parser.write_some(__buf_recive + count_byte_write);
			}
			catch (exception& e)
			{
				cerr << e.what() << endl;
			}
			if (!__parser.done())
			{
				fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);
				cerr << "Json is not full" << endl;
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
				return;
			}
			try
			{
				__buf_json_recive.push(__parser.release());
				__parser.reset();
				count_byte_write += count_byte;
			}
			catch (exception& e)
			{
				__parser.reset();
				__buf_json_recive = {};
				cerr << "Json read " << e.what() << endl;
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__reciveCommand, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
				return;
			}
		}
		fill_n(__buf_recive, BUF_RECIVE_SIZE, 0);

		while(__buf_json_recive.size() != 0)
		{
			try
			{
				__json_string = __buf_json_recive.front();
				__buf_json_recive.pop();

				//cerr << json_string << endl;
				boost::json::value target = __json_string.at("target");
				if (target == "ping")
				{
					__buf_send = boost::json::serialize(json_formatter::worker::response::ping(__name));
				}
				else if (target == "disconnect")
				{
					__buf_send = boost::json::serialize(json_formatter::worker::response::disconnect(__name));
					__flag_disconnect = true;
				}
				else if (target == "marussia_station_request")
				{
					__analizeRequest();
				}
				cerr << __flag_disconnect << endl;
				if (__buf_json_recive.size() == 0)
				{
					__flag_end_send = true;
				}
				__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__reciveAnswer, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));

			}
			catch (exception& e)
			{
				cerr << e.what() << endl;
				__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__sendResponse, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
		}
	}

	void __reciveAnswer(const boost::system::error_code& eC, size_t bytes_send)
	{
		if (eC)
		{
			cerr << "recieveCommand " << eC.message() << endl;
			__socket->async_send(net::buffer(__buf_send, __buf_send.size()), boost::bind(&Worker::__reciveAnswer, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		static size_t temp_send = 0;
		temp_send += bytes_send;
		if (temp_send != __buf_send.size())
		{
			__log->writeTempLog(0, "DataBase", "Not_full_json");
			__socket->async_send(net::buffer(__buf_send.c_str() + temp_send, __buf_send.size() - temp_send), boost::bind(&Worker::__reciveAnswer, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			return;
		}
		temp_send = 0;
		__buf_send.clear();
		if (__flag_disconnect)
		{
			__flag_disconnect = false;
			this->stop();
		}
		else if (__flag_end_send)
		{
			__flag_end_send = false;
			__socket->async_receive(net::buffer(__buf_recive, BUF_RECIVE_SIZE), boost::bind(&Worker::__sendResponse, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		else
		{
			cerr << "no one send " << endl;
		}
	}

	void __connectToBd()
	{

		queue<string> tables;
		tables.push("MarussiaStation"); tables.push("House"); tables.push("StaticPhrases");
		queue<vector<string>> fields;
		vector<string> fields_marussia = { "ApplicationId", "HouseComplexId", "HouseNum", "LiftBlockId"};
		vector<string> fields_house = { "TopFloor", "BottomFloor", "NullFloor", "HouseNumber", "HousingComplexId" };
		vector<string> fields_phrases = { "HouseComplexId", "HouseNum", "KeyWords", "Response" };
		fields.push(fields_marussia); fields.push(fields_house); fields.push(fields_phrases);

		queue<string> conditions;
		//conditions.push("WHERE WorkerId = \"1\" OR WokerSecId = \"1\""); conditions.push(""); conditions.push("");
		cout << __db_log << " " << __db_pas << endl;
		cout << __ip_db << " " << __port_db << endl;
		__db_client->setQuerys(tables, fields, conditions);
		__db_client->start();
	}

	void __resetTimer()
	{
		__log->writeTempLog(0, __name, "__rewrite_data_base");
		__connectToBd();
		__timer->expires_from_now(boost::posix_time::hours(24));
		__timer->async_wait(boost::bind(&Worker::__resetTimer, shared_from_this()));
	}

	void __analizeRequest()
	{
		__log->writeTempLog(0, __name, "__analize_response");
		///___marussia station House number and complex id___///
		string app_id = boost::json::serialize(__json_string.at("request").at("station_id"));


		cerr << "app_id" << app_id << endl;
		map<string, vector<string>> one_table = __db_info.at("MarussiaStation");
		vector<string> buf_vec = __db_info.at("MarussiaStation").at("ApplicationId");
		vector<string> house_vec = __db_info.at("MarussiaStation").at("HouseNum");
		vector<string> comp_vec = __db_info.at("MarussiaStation").at("HouseComplexId");
		vector<string> lift_vec = __db_info.at("MarussiaStation").at("LiftBlockId");

		string num_house = "-1";
		string comp_id = "-1";
		string lift_block = "";
		__response_command.clear();

		for (size_t i = 0; i < buf_vec.size(); i++)
		{
			if (app_id == buf_vec[i])
			{
				cerr << "num_house " << num_house << endl << "comp_id " << comp_id << endl << "lift_block " << lift_block << endl;
				num_house = house_vec[i];
				comp_id = comp_vec[i];
				lift_block = lift_vec[i];
				break;
			}
		}
		if (num_house == "-1" || comp_id == "-1")
		{
			cerr << "error no House or comp" << endl;
			this->stop();
		}
		//cerr << "num_house " << num_house << endl;
		//cerr << "comp_id " << comp_id << endl;
		//cerr << "lift_block " << lift_block << endl;

		///___search for mqtt_____/////
		boost::json::array array_tokens = __json_string.at("request").at("body").at("request").at("nlu").at("tokens").as_array();
		vector<string> search_mqtt;
		bool flag_mqtt = false;
		boost::locale::generator gen;
		for (size_t i = 0; i < array_tokens.size(); i++)
		{

			string buf_string_mqtt = boost::json::serialize(array_tokens[i]);
			string string_mqtt;
			remove_copy(buf_string_mqtt.begin(), buf_string_mqtt.end(), back_inserter(string_mqtt), '"');
			search_mqtt.push_back(string_mqtt);
			for(size_t j = 0; j < __mqtt_keys.size(); j++)
			{
				string buf_mqtt_key = boost::locale::conv::to_utf<char>(string(__mqtt_keys[j].begin(), __mqtt_keys[j].end()), gen(""));
				cerr << "array" << string_mqtt << " my" << buf_mqtt_key << endl;
				if (string_mqtt == buf_mqtt_key)
				{
					flag_mqtt = true;
				}
			}
		}
		cerr << "flag_mqtt " << flag_mqtt << endl;
		if (flag_mqtt)
		{
			one_table.clear();
			one_table = __db_info.at("House");
			buf_vec.clear(); house_vec.clear(); comp_vec.clear();
			house_vec = __db_info.at("House").at("HouseNumber");
			comp_vec = __db_info.at("House").at("HousingComplexId");
			vector<string> top_floor = __db_info.at("House").at("TopFloor");
			vector<string> bot_floor = __db_info.at("House").at("BottomFloor");
			vector<string> null_floor = __db_info.at("House").at("NullFloor");
			string bufNum = "";
			for (size_t i = 0; i < __key_roots.size(); i++)
			{
				for (size_t j = 0; j < search_mqtt.size(); j++)
				{
					string buf_key = boost::locale::conv::to_utf<char>(string(__key_roots[i].begin(), __key_roots[i].end()), gen(""));
					cerr << search_mqtt[j] << " my" << buf_key << endl;
					if (search_mqtt[j].find(buf_key) != search_mqtt[j].npos)
					{
						bufNum += search_mqtt[j];
					}
				}
			}
			cerr << "bufNum " << bufNum << endl;
			u8string string_number;
			remove_copy(bufNum.begin(), bufNum.end(), back_inserter(string_number), '"');
			int numFloor = __num_roots.at(string_number);
			cerr << numFloor  << endl;
			for (size_t i = 0; i < house_vec.size(); i++)
			{
				if (house_vec[i] == num_house && comp_vec[i] == comp_id)
				{
					string boofer;
					remove_copy(top_floor[i].begin(), top_floor[i].end(), back_inserter(boofer), '"');
					int top = stoi(boofer);

					boofer.clear();
					remove_copy(bot_floor[i].begin(), bot_floor[i].end(), back_inserter(boofer), '"');
					int bot = stoi(boofer);

					boofer.clear();
					remove_copy(null_floor[i].begin(), null_floor[i].end(), back_inserter(boofer), '"');
					int null;
					if (boofer == "-1")
					{
						null = -1;
					}
					else
					{
						null = 0;
					}
					//cerr << top << "" << bot << " " << null << endl;
 					if (top >= numFloor && bot <= numFloor || null == numFloor)
					{
						//__response_command = "перемещаю вас на " + numFloor + "этаж";
						u8string buf_u8_resp = u8"Перемещаю вас на ";
						__response_command = boost::locale::conv::to_utf<char>(string(buf_u8_resp.begin(), buf_u8_resp.end()), gen(""));
						__response_command += bufNum;
						buf_u8_resp = u8" этаж";
						__response_command += boost::locale::conv::to_utf<char>(string(buf_u8_resp.begin(), buf_u8_resp.end()), gen(""));
						break;
					}
				}
			}
			cerr << "mqtt ";
			cerr << __response_command << endl;
			__buf_send = boost::json::serialize(json_formatter::worker::response::marussia_mqtt_message(__name, app_id, __getRespToMS(__response_command), lift_block, numFloor));
			//cerr << __buf_send << endl;
			flag_mqtt = false;
		}
		else
		{
			///_____static phrases response static phrase___///

			string buf_command = boost::json::serialize(__json_string.at("request").at("body").at("request").at("command"));
			string command;
			remove_copy(buf_command.begin(), buf_command.end(), back_inserter(command), '"');
			one_table.clear();
			one_table = __db_info.at("StaticPhrases");
			buf_vec.clear(); house_vec.clear(); comp_vec.clear();
			buf_vec = __db_info.at("StaticPhrases").at("KeyWords");
			house_vec = __db_info.at("StaticPhrases").at("HouseNum");
			comp_vec = __db_info.at("StaticPhrases").at("HouseComplexId");
			vector<string> resp = __db_info.at("StaticPhrases").at("Response");
			bool flag_stop_search = false;

			for (size_t i = 0; i < buf_vec.size(); i++)
			{
				string all_variants = buf_vec[i];
				vector<string> vector_variants;
				int num = all_variants.find(";");
				if (num != all_variants.npos)
				{
					while (num != all_variants.npos)
					{
						string buf_string;
						buf_string.assign(all_variants, 0, num);
						all_variants.erase(0, buf_string.size() + 1);
						vector_variants.push_back(buf_string);
						num = all_variants.find(";");
					}
					vector_variants.push_back(all_variants);
				}
				else
				{
					vector_variants.push_back(all_variants);
				}
				for(size_t j = 0; j < vector_variants.size(); j++)
				{
					string buf_variant;
					remove_copy(vector_variants[j].begin(), vector_variants[j].end(), back_inserter(buf_variant), '"');
					cerr << command << " " << buf_variant << endl;
					if (command.find(buf_variant)!= string::npos)
					{
						cerr << command << " " << buf_variant << endl;
						if (num_house == house_vec[i] && comp_id == comp_vec[i])
						{
							cerr << "resp" << resp[i] << endl;
							__response_command = resp[i];
							flag_stop_search = true;
							break;
						}
					}
				}
				if (flag_stop_search)
				{
					flag_stop_search = false;
					break;
				}
			}
			if (!__response_command.empty())
			{
				__buf_send = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, app_id, __getRespToMS(__response_command)));
				cerr << __buf_send << endl;
			}
			else
			{
				u8string buf_resp_u8 = u8"Извините, я не знаю такой команды, пожалуйста, перефразируйте";
				__response_command.clear();
				__response_command = boost::locale::conv::to_utf<char>(string(buf_resp_u8.begin(), buf_resp_u8.end()), gen(""));
				__buf_send = boost::json::serialize(json_formatter::worker::response::marussia_static_message(__name, app_id, __getRespToMS(__response_command)));
				cerr << __buf_send << endl;
			}
		}

	}
	boost::json::object __getRespToMS(string respText)
	{
		return boost::json::object({
									{"text", respText},
									{"tts", respText},
									{"end_session", false}
									});
	}


	enum __CHECK_STATUS
	{
		SUCCESS = 1,
		FAIL
	};

	__CHECK_STATUS __checkSend(const size_t& count_send_byte, size_t& temp_send_byte, __handler_t&& handler);
	__CHECK_STATUS __checkJson(const size_t& countReciveByte, __handler_t&& handler);

};
//-------------------------------------------------------------//

