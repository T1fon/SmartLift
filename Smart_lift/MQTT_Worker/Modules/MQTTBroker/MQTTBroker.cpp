#include "MQTTBroker.hpp"
//(*end_p).publish("LU127728/set/cmd", std::string("{\"cmdlft\":[1,") + std::to_string(a.at(i)) + std::string("]}"));
using namespace mqtt_broker;
MQTTBroker::MQTTBroker() {}
MQTTBroker::MQTTBroker(shared_ptr<MQTT_NS::server<>> server):__server(server){}

void MQTTBroker::__closeProc(std::set<con_sp_t>& cons, mi_sub_con& subs, con_sp_t const& con) {
    cons.erase(con);

    auto& idx = subs.get<tag_con>();
    auto r = idx.equal_range(con);
    idx.erase(r.first, r.second);
}
void MQTTBroker::setServer(shared_ptr<MQTT_NS::server<>> server) {
    __server = server;
}
void MQTTBroker::init(){
    
    __server->set_error_handler(
        [](MQTT_NS::error_code ec) {
            std::cout << "error: " << ec.message() << std::endl;
        }
    );
    
    __server->set_accept_handler(
        [this](con_sp_t spep) {
            auto& ep = *spep;
            __wp = spep;
            __worker = spep;
            std::cout << "accept" << std::endl;
            
            // Pass spep to keep lifetime.
            // It makes sure wp.lock() never return nullptr in the handlers below
            // including close_handler and error_handler.
            ep.start_session(std::move(spep));

            // set connection (lower than MQTT) level handlers
            ep.set_close_handler(
                [this]
            () {
                    std::cout << "[server] closed." << std::endl;
                    auto sp = __wp.lock();
                    BOOST_ASSERT(sp);
                    __closeProc(__connections, __subs, sp);
                });
            ep.set_error_handler(
                [this]
            (MQTT_NS::error_code ec) {
                    std::cout << "[server] error: " << ec.message() << std::endl;
                    auto sp = __wp.lock();
                    BOOST_ASSERT(sp);
                    __closeProc(__connections, __subs, sp);
                });

            // set MQTT level handlers
            ep.set_connect_handler(
                [this]
            (MQTT_NS::buffer client_id,
                MQTT_NS::optional<MQTT_NS::buffer> username,
                MQTT_NS::optional<MQTT_NS::buffer> password,
                MQTT_NS::optional<MQTT_NS::will>,
                bool clean_session,
                std::uint16_t keep_alive) {
                    using namespace MQTT_NS::literals;
                    std::cout << "[server] client_id    : " << client_id << std::endl;
                    std::cout << "[server] username     : " << (username ? username.value() : "none"_mb) << std::endl;
                    std::cout << "[server] password     : " << (password ? password.value() : "none"_mb) << std::endl;
                    std::cout << "[server] clean_session: " << std::boolalpha << clean_session << std::endl;
                    std::cout << "[server] keep_alive   : " << keep_alive << std::endl;
                    try {
                        (*__sp_db_map_login_password)->at("\"" + username->to_string() + "\"");
                        string temp_client_id = client_id.data();
                        static const string SEARCH_PHRASE = "LKDS_LU";
                        static const int SPACE = 32;
                        static const int SIZE_SEARCH_PHRASE = sizeof(SEARCH_PHRASE);
                        size_t pos_lkds_lu = temp_client_id.find(SEARCH_PHRASE);
                        size_t pos_number_space = temp_client_id.find(SPACE, pos_lkds_lu + SIZE_SEARCH_PHRASE);
                        __lu_blocks_id.push_back(temp_client_id.substr(pos_lkds_lu, pos_number_space));
                        auto sp = __wp.lock();
                        BOOST_ASSERT(sp);
                        __connections.insert(sp);
                        sp->connack(false, MQTT_NS::connect_return_code::accepted);
                        return true;
                    }
                    catch (exception &e) {
                        cout << "CLIENT NOT AUTHORIZED" << endl;
                        auto sp = __wp.lock();
                        BOOST_ASSERT(sp);
                        sp->connack(false, MQTT_NS::connect_return_code::not_authorized);
                        return true;
                    }
                    
                }
            );
            ep.set_disconnect_handler(
                [this]
            () {
                    std::cout << "[server] disconnect received." << std::endl;
                    auto sp = __wp.lock();
                    BOOST_ASSERT(sp);
                    __closeProc(__connections, __subs, sp);
                });
            ep.set_puback_handler(
                []
            (packet_id_t packet_id) {
                    std::cout << "[server] puback received. packet_id: " << packet_id << std::endl;
                    return true;
                });
            ep.set_pubrec_handler(
                []
            (packet_id_t packet_id) {
                    std::cout << "[server] pubrec received. packet_id: " << packet_id << std::endl;
                    return true;
                });
            ep.set_pubrel_handler(
                []
            (packet_id_t packet_id) {
                    std::cout << "[server] pubrel received. packet_id: " << packet_id << std::endl;
                    return true;
                });
            ep.set_pubcomp_handler(
                []
            (packet_id_t packet_id) {
                    std::cout << "[server] pubcomp received. packet_id: " << packet_id << std::endl;
                    return true;
                });
            ep.set_publish_handler(
                [this]
            (MQTT_NS::optional<packet_id_t> packet_id,
                MQTT_NS::publish_options pubopts,
                MQTT_NS::buffer topic_name,
                MQTT_NS::buffer contents) {
                    std::cout << "[server] publish received."
                        << " dup: " << pubopts.get_dup()
                        << " qos: " << pubopts.get_qos()
                        << " retain: " << pubopts.get_retain() << std::endl;
                    if (packet_id)
                        std::cout << "[server] packet_id: " << *packet_id << std::endl;
                    std::cout << "[server] topic_name: " << topic_name << std::endl;
                    std::cout << "[server] contents: " << contents << std::endl;
                    auto const& idx = __subs.get<tag_topic>();
                    auto r = idx.equal_range(topic_name);
                    for (; r.first != r.second; ++r.first) {
                        r.first->con->publish(
                            topic_name,
                            contents,
                            std::min(r.first->qos_value, pubopts.get_qos())
                        );
                    }
                    return true;
                });
            ep.set_subscribe_handler(
                [this]
            (packet_id_t packet_id,
                std::vector<MQTT_NS::subscribe_entry> entries) {
                    std::cout << "[server] subscribe received. packet_id: " << packet_id << std::endl;
                    std::vector<MQTT_NS::suback_return_code> res;
                    res.reserve(entries.size());
                    auto sp = __wp.lock();
                    BOOST_ASSERT(sp);
                    for (auto const& e : entries) {
                        std::cout << "[server] topic_filter: " << e.topic_filter << " qos: " << e.subopts.get_qos() << std::endl;
                        res.emplace_back(MQTT_NS::qos_to_suback_return_code(e.subopts.get_qos()));
                        __subs.emplace(std::move(e.topic_filter), sp, e.subopts.get_qos());
                    }
                    sp->suback(packet_id, res);
                    return true;
                }
            );
            ep.set_unsubscribe_handler(
                [this]
            (packet_id_t packet_id,
                std::vector<MQTT_NS::unsubscribe_entry> entries) {
                    std::cout << "[server] unsubscribe received. packet_id: " << packet_id << std::endl;
                    auto sp = __wp.lock();
                    for (auto const& e : entries) {
                        auto it = __subs.find(std::make_tuple(sp, e.topic_filter));
                        if (it != __subs.end()) {
                            __subs.erase(it);
                        }
                    }
                    BOOST_ASSERT(sp);
                    sp->unsuback(packet_id);
                    return true;
                }
            );
            ep.set_pingresp_handler([]()
                {
                    std::cout << "[server] pingresp" << std::endl;
                    return true;
                });

        }
    );
}
void MQTTBroker::start(shared_ptr<shared_ptr<map<string, string>>> sp_db_map_login_password){
    __sp_db_map_login_password = sp_db_map_login_password;
    for (auto i = (*__sp_db_map_login_password)->begin(); i != (*__sp_db_map_login_password)->end(); i++) {
        cout << i->first << " " << i->second << endl;
    }
    __server->listen();
}
bool MQTTBroker::searchLiftBlocks(string lu_descriptor){
    for(auto i = __lu_blocks_id.begin(), end = __lu_blocks_id.end(); i != end; i++){
        if(*i == lu_descriptor){
            return true;
        }
    }
    return false;
}
void MQTTBroker::stop(){
    __server->close();
}
MQTTBroker::~MQTTBroker() {
    this->stop();
}
con_sp_t MQTTBroker::getWorker() {
    return __worker;
}