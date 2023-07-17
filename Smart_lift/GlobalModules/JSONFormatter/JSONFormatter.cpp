#include "JSONFormatter.hpp"
using namespace json_formatter;
using namespace boost;
using namespace std;

json::object worker::request::ping(string sender) {
	return json::object({ { "sender", sender}, {"target", "ping"} });
}
json::object worker::request::connect(string sender, string id) {
	return json::object({ { "sender", sender}, {"target", "connect"}, {"request", {"id", id}}});
}
json::object worker::request::disconnect(string sender) {
	return json::object({ { "sender", sender}, {"target", "disconnect"} });
}
json::object worker::request::marussia_request(string sender, string station_id, json::value body) {
	return json::object({	{ "sender", sender},
							{ "target", "Marussia_station_request"}, 
							{ "request", {
									{"station_id", station_id},
									{"body", body}
								}
							}
						});
}
json::object worker::request::mqtt_move(string sender, string station_id, string lift_block_id, int floor) {
	return json::object({ { "sender", sender},
							{ "target", "Move_lift"},
							{ "request", {
									{"station_id", station_id},
									{"MQTT_command", {
										{"lb_id", lift_block_id},
										{"value", to_string(floor)}
										}
									}
								}
							}
						});
}

json::object worker::response::ping(string sender) {
	return json::object({ {"sender", sender}, {"target", "ping"}, { "response", {"status", "success"}}});
}
json::object worker::response::connect(string sender){
	return json::object({ {"sender", sender}, {"target", "connect"}, { "response", {"status", "success"}} });
}
json::object worker::response::disconnect(string sender){
	return json::object({ { "sender", sender}, {"target", "disconnect"}, { "response", {"status", "success"}} });
}
json::object worker::response::connect(string sender, ERROR_CODE err_code, string err_message){
	return json::object({	{"sender", sender}, 
							{"target", "connect"}, 
							{ "response", {
									{"status", "fail"},
									{"error_code", to_string(err_code)},
									{"message", err_message}
								}
							} 
						});
}
json::object worker::response::disconnect(string sender, ERROR_CODE err_code, string err_message){
	return json::object({ {"sender", sender},
							{"target", "disconnect"},
							{ "response", {
									{"status", "fail"},
									{"error_code", to_string(err_code)},
									{"message", err_message}
								}
							}
						});
}
json::object worker::response::marussia_static_message(string sender, string station_id, json::object response_body){
	return json::object({	{"sender", sender},
							{"target", "Static_message"},
							{"response", {
								{"station_id", station_id},
								{"response_body", response_body}
								}
							}
						});
}
json::object worker::response::marussia_mqtt_message(string sender, string station_id, json::object response_body, string lift_block_id, int floor){
	return json::object({ {"sender", sender},
							{"target", "Move_lift"},
							{"response", {
								{"station_id", station_id},
								{"response_body", response_body},
								{"MQTT_command", {
									{"lb_id", lift_block_id},
									{"value", to_string(floor)}
									}
								}
								}
							}
		});
}
json::object worker::response::mqtt_lift_move(string sender, string station_id, STATUS_OPERATION status, string err_message){
	
	switch (status) {
	case STATUS_OPERATION::success:
		return json::object({	{"sender", sender},
								{"target", "MQTT_message"},
								{"response", {
									{"station_id", station_id},
									{"status", "success"}
									}
								}
							});
		break;
	case STATUS_OPERATION::fail:
		return json::object({ {"sender", sender},
								{"target", "MQTT_message"},
								{"response", {
									{"station_id", station_id},
									{"status", "fail"},
									{"message", err_message},
									}
								}
			});
		break;
	default:
		return {};
		break;
	}
}


json::object database::request::ping(string sender){
	return json::object({ { "sender", sender}, {"target", "ping"} });
}
json::object database::request::connect(string sender, string login, string password){
	return json::object({	{"sender", sender}, 
							{"target", "connect"}, 
							{"request", {
								{"login", login},
								{"password", password},
								}
							}
						});
}
json::object database::request::disconnect(string sender){
	return worker::request::disconnect(sender);
}
json::object database::request::query(string sender, QUERY_METHOD method, vector<string> fields, string query){
	json::array json_fields;
	for (size_t i = 0, length = fields.size(); i < length; ++i) {
		json_fields.emplace_back(fields[i]);
	}
	switch (method) {
	case SELECT:
		return json::object({	{"sender", sender},
								{"target", "DB_query"},
								{"request", {
									{"method", "select"},
									{"fields", json_fields},
									{"query", query}
									}
								}
							});
		break;
	default:
		return {};
	}
}

json::object database::response::ping(string sender){
	return worker::response::ping(sender);
}
json::object database::response::connect(string sender){
	return worker::response::connect(sender);
}
json::object database::response::disconnect(string sender){
	return worker::response::disconnect(sender);
}
json::object database::response::connect(string sender, ERROR_CODE err_code, string err_message){
	return worker::response::connect(sender, err_code, err_message);
}
json::object database::response::disconnect(string sender, ERROR_CODE err_code, string err_message){
	return worker::response::disconnect(sender, err_code, err_message);
}
json::object database::response::query(string sender, QUERY_METHOD method, map< string, vector<string> > fields){
	json::object result;
	json::object response;
	result["sender"] = sender;
	result["target"] = "DB_query";
	switch (method) {
		case SELECT:
			response["method"] = "select";
			break;
	}
	json::array value;
	for (auto i = fields.begin(); i != fields.end(); i++) {
		value = {};
		for (size_t j = 0, length = (*i).second.size(); j < length; j++) {
			value.emplace_back((*i).second[j]);
		}
		response[(*i).first] = value;
	}
	result["response"] = response;
	return result;
}