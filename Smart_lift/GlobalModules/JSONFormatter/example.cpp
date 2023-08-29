#include "boost/json.hpp"
#include "JSONFormatter.hpp"
#include <iostream>

using namespace std;
int main(int argc, char** argv) {

    cout << "Worker" << endl << endl;
    cout << "Request" << endl;
    cout << json_formatter::worker::request::ping("Main_Server") << endl;
    cout << json_formatter::worker::request::connect("Worker_Marussia", "5") << endl;
    cout << json_formatter::worker::request::disconnect("Main_Server") << endl;
    cout << json_formatter::worker::request::marussia_request("Main_Server", "123456123456", {}) << endl;
    cout << json_formatter::worker::request::mqtt_move("Main_Server", "123456123456", "LU911", 5) << endl;

    cout << "Response" << endl;
    cout << json_formatter::worker::response::ping("Worker_MQTT") << endl;
    cout << json_formatter::worker::response::connect("Main_Server") << endl;
    cout << json_formatter::worker::response::connect("Main_Server", json_formatter::ERROR_CODE::CONNECT, "Worker not found") << endl;
    cout << json_formatter::worker::response::disconnect("Main_Server") << endl;
    cout << json_formatter::worker::response::disconnect("Main_Server", json_formatter::ERROR_CODE::DISCONNECT, "Operation not end") << endl;
    cout << json_formatter::worker::response::marussia_static_message("Worker_Marussia",
        "123456654321",
        json::object({ { "text", "bebebe" } })) << endl;
    cout << json_formatter::worker::response::marussia_mqtt_message("Worker_Marussia",
        "123456654321",
        json::object({ { "text", "bebebe" } }),
        "LU911",
        5) << endl;
    cout << json_formatter::worker::response::mqtt_lift_move("Worker_MQTT", "LU911", json_formatter::STATUS_OPERATION::success) << endl;
    cout << json_formatter::worker::response::errorTarget("Data_base", boost::json::value("bla bla"), json_formatter::ERR_CODE::TARGET, "") << endl;

    cout << endl << endl << endl;
    cout << "Data Base" << endl << endl;
    cout << "Request" << endl;
    cout << json_formatter::database::request::ping("Main_Server") << endl;
    cout << json_formatter::database::request::connect("Main_Server", "root", "root") << endl;
    cout << json_formatter::database::request::disconnect("MainServer") << endl;
    cout << json_formatter::database::request::query("Main_Server",
        json_formatter::database::QUERY_METHOD::SELECT,
        { "id", "name" },
        "SELECT * FROM Country") << endl;
    cout << "Response" << endl;
    cout << json_formatter::database::response::ping("Data_base") << endl;
    cout << json_formatter::database::response::connect("Data_base") << endl;
    cout << json_formatter::database::response::connect("Data_base", json_formatter::ERROR_CODE::CONNECT, "User not found") << endl;
    cout << json_formatter::database::response::disconnect("Data_base") << endl;
    cout << json_formatter::database::response::disconnect("Data_base", json_formatter::ERROR_CODE::DISCONNECT, "Operation not end") << endl;
    cout << json_formatter::database::response::query("Data_base",
        json_formatter::database::QUERY_METHOD::SELECT,
        { {"id",{"1","2","3"}} , { "name", {"Russia", "England", "USA"}} }) << endl;
    cout << json_formatter::database::response::errorTarget("Data_base", boost::json::value("bla bla"), json_formatter::ERR_CODE::TARGET, "") << endl;
    return 0;

}