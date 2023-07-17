#pragma once
#include <string>
#include <vector>
#include <map>
#include <boost/json.hpp>

using namespace boost;
using namespace std;

namespace json_formatter{

	enum ERROR_CODE {
		CONNECT = 3,
		DISCONNECT,
	};
	enum STATUS_OPERATION{
		success = 1,
		fail
	};
	namespace worker {
		namespace request {
			/*ОБЩИЕ*/
			json::object ping(string sender);
			json::object connect(string sender, string id);
			json::object disconnect(string sender);

			/*ГС*/
			json::object marussia_request(string sender, string station_id, json::value body);
			json::object mqtt_move(string sender, string station_id, string lift_block_id, int floor);
		}
		namespace response {
			/*ОБЩИЕ*/
			json::object ping(string sender);
			//ответит success
			json::object connect(string sender);
			json::object disconnect(string sender);
			//ответит с кодом ошибки
			json::object connect(string sender, ERROR_CODE err_code, string err_message = "");
			json::object disconnect(string sender, ERROR_CODE err_code, string err_message = "");

			/*marussia worker*/
			json::object marussia_static_message(string sender, string station_id, json::object response_body);
			json::object marussia_mqtt_message(string sender, string station_id, json::object response_body, string lift_block_id, int floor);
			/*mqtt worker*/
			json::object mqtt_lift_move(string sender, string station_id, STATUS_OPERATION status, string err_message = "");
		}
		
	}
	namespace database {
		enum QUERY_METHOD {
			SELECT = 1
		};
		namespace request {
			json::object ping(string sender);
			json::object connect(string sender, string login, string password);
			json::object disconnect(string sender);
			json::object query(string sender, QUERY_METHOD method, vector<string> fields, string query);
		}
		namespace response {
			json::object ping(string sender);
			//ответит success
			json::object connect(string sender);
			json::object disconnect(string sender);
			//ответит с кодом ошибки
			json::object connect(string sender, ERROR_CODE err_code, string err_message = "");
			json::object disconnect(string sender, ERROR_CODE err_code, string err_message = "");
			json::object query(string sender, QUERY_METHOD method, map< string, vector<string> > fields);
		}
	}
}