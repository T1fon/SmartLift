#include "MQTTBroker.hpp"
//(*end_p).publish("LU127728/set/cmd", std::string("{\"cmdlft\":[1,") + std::to_string(a.at(i)) + std::string("]}"));
using namespace mqtt_broker;

MQTTBrokerSession::MQTTBrokerSession(con_sp_t end_point, shared_ptr<set<con_sp_t>> connections, shared_ptr<mi_sub_con> subs):__end_point(*end_point){
    __connections = connections;
    __subs = subs;
    __worker = end_point;
    __move_worker = end_point;
    __is_live = true;
    __wp = __worker;
    __name = "";
}
MQTTBrokerSession::~MQTTBrokerSession(){
    __is_live = false;
}
bool MQTTBrokerSession::isLive(){
    return __is_live;
}
void MQTTBrokerSession::setName(string name){
    __name = name;
}
string MQTTBrokerSession::getName(){
    return __name;
}

void MQTTBrokerSession::__closeProc(std::set<con_sp_t>& cons, mi_sub_con& subs, con_sp_t const& con) {
    __name = "";
    __is_live = false;
    cons.erase(con);

    auto& idx = subs.get<tag_con>();
    auto r = idx.equal_range(con);
    idx.erase(r.first, r.second);
}

void MQTTBrokerSession::init(shared_ptr<shared_ptr<map<string, string>>> sp_db_map_login_password){
    __sp_db_map_login_password = sp_db_map_login_password;
    __end_point.start_session(std::move(__move_worker));
        
    // set connection (lower than MQTT) level handlers
    using packet_id_t = typename std::remove_reference_t<decltype(__end_point)>::packet_id_t;
    __end_point.set_close_handler(
        [this]
    () {    
            std::cout << __name << " [server] closed." << std::endl;
            try{
                auto sp = __wp.lock();
                BOOST_ASSERT(sp);
                __closeProc(*__connections, *__subs, sp);
            }catch(exception &e){
                std::cerr << e.what() << std::endl;
            }
        });
    __end_point.set_error_handler(
        [this]
    (MQTT_NS::error_code ec) {
            std::cout <<__name << " [server] error: " << ec.message()<< " " << ec << std::endl;
            try{
                auto sp = __wp.lock();
                BOOST_ASSERT(sp);
                __closeProc(*__connections, *__subs, sp);
            }catch(exception &e){
                std::cerr << e.what() << std::endl;
            }
            
        });

    // set MQTT level handlers
    __end_point.set_connect_handler(
        [this]
    (MQTT_NS::buffer client_id,
        MQTT_NS::optional<MQTT_NS::buffer> username,
        MQTT_NS::optional<MQTT_NS::buffer> password,
        MQTT_NS::optional<MQTT_NS::will>,
        bool clean_session,
        std::uint16_t keep_alive) {
            using namespace MQTT_NS::literals;
            //std::cout << "[server] client_id    : " << client_id << std::endl;
            //std::cout << "[server] username     : " << (username ? username.value() : "none"_mb) << std::endl;
            //std::cout << "[server] password     : " << (password ? password.value() : "none"_mb) << std::endl;
            //std::cout << "[server] clean_session: " << std::boolalpha << clean_session << std::endl;
            //std::cout << "[server] keep_alive   : " << keep_alive << std::endl;
            try {
                (*__sp_db_map_login_password)->at("\"" + username->to_string() + "\"");
                string temp_client_id = client_id.data();
                static const string SEARCH_PHRASE = "LKDS_LU";
                static const int SPACE = 32;
                static const int SIZE_SEARCH_PHRASE = SEARCH_PHRASE.size();
                size_t pos_lkds_lu = temp_client_id.find(SEARCH_PHRASE);
                size_t pos_number_space = temp_client_id.find(SPACE, pos_lkds_lu + SIZE_SEARCH_PHRASE);
                cout <<"NEW LU: "<< temp_client_id.substr(pos_lkds_lu + SIZE_SEARCH_PHRASE, pos_number_space - SIZE_SEARCH_PHRASE) << " " << pos_number_space<< endl;
                __name = temp_client_id.substr(pos_lkds_lu + SIZE_SEARCH_PHRASE, pos_number_space - SIZE_SEARCH_PHRASE);
                auto sp = __wp.lock();
                BOOST_ASSERT(sp);
                __connections->insert(sp);
                sp->connack(false, MQTT_NS::connect_return_code::accepted);
                return true;
            }
            catch (exception &e) {
                __is_live = false;
                cout << "CLIENT " << username->to_string()  <<" NOT AUTHORIZED" << endl;
                //auto sp = __wp.lock();
                //BOOST_ASSERT(sp);
                //__closeProc(*__connections, *__subs, sp);
                //sp->connack(false, MQTT_NS::connect_return_code::not_authorized);
                //мб поставить falseeeeeee
                return false;
            }
            
        }
    );
    __end_point.set_disconnect_handler(
        [this]
        () {
            std::cout <<__name << " [server] disconnect received." << std::endl;
            auto sp = __wp.lock();
            BOOST_ASSERT(sp);
            __closeProc(*__connections, *__subs, sp);
        });
    __end_point.set_puback_handler(
        []
    (packet_id_t packet_id) {
            //std::cout << "[server] puback received. packet_id: " << packet_id << std::endl;
            return true;
        });
    __end_point.set_pubrec_handler(
        []
    (packet_id_t packet_id) {
            //std::cout << "[server] pubrec received. packet_id: " << packet_id << std::endl;
            return true;
        });
    __end_point.set_pubrel_handler(
        []
    (packet_id_t packet_id) {
            //std::cout << "[server] pubrel received. packet_id: " << packet_id << std::endl;
            return true;
        });
    __end_point.set_pubcomp_handler(
        []
    (packet_id_t packet_id) {
            //std::cout << "[server] pubcomp received. packet_id: " << packet_id << std::endl;
            return true;
        });
    __end_point.set_publish_handler(
        [this]
    (MQTT_NS::optional<packet_id_t> packet_id,
        MQTT_NS::publish_options pubopts,
        MQTT_NS::buffer topic_name,
        MQTT_NS::buffer contents) {
            //std::cout << "PUBLISH START" << std::endl;
            ///////////////////////////std::cout << "[server] publish received."
                //<< " dup: " << pubopts.get_dup()
                //<< " qos: " << pubopts.get_qos()
                //<< " retain: " << pubopts.get_retain() << std::endl;
            //if (packet_id)
                //std::cout << "[server] packet_id: " << *packet_id << std::endl;
            std::cout << __name <<" [server] topic_name: " << topic_name << std::endl;
            std::cout << __name <<" [server] contents: " << contents << std::endl;
            
            //std::cout << "PUBLISH END" << std::endl;
            return true;
        });
    __end_point.set_subscribe_handler(
        [this]
    (packet_id_t packet_id,
        std::vector<MQTT_NS::subscribe_entry> entries) {
            //std::cout << "[server] subscribe received. packet_id: " << packet_id << std::endl;
            std::vector<MQTT_NS::suback_return_code> res;
            res.reserve(entries.size());
            auto sp = __wp.lock();
            BOOST_ASSERT(sp);
            for (auto const& e : entries) {
              //  std::cout << "[server] topic_filter: " << e.topic_filter << " qos: " << e.subopts.get_qos() << std::endl;
                res.emplace_back(MQTT_NS::qos_to_suback_return_code(e.subopts.get_qos()));
                __subs->emplace(std::move(e.topic_filter), sp, e.subopts.get_qos());
            }
            sp->suback(packet_id, res);
            return true;
        }
    );
    __end_point.set_unsubscribe_handler(
        [this]
    (packet_id_t packet_id,
        std::vector<MQTT_NS::unsubscribe_entry> entries) {
            //std::cout << "[server] unsubscribe received. packet_id: " << packet_id << std::endl;
            auto sp = __wp.lock();
            for (auto const& e : entries) {
                auto it = __subs->find(std::make_tuple(sp, e.topic_filter));
                if (it != __subs->end()) {
                    __subs->erase(it);
                }
            }
            BOOST_ASSERT(sp);
            sp->unsuback(packet_id);
            return true;
        }
    );
    __end_point.set_pingresp_handler([]()
        {
            //std::cout << "[server] pingresp" << std::endl;
            return true;
        });
}
void MQTTBrokerSession::publish(string topic,string message){
    __worker->async_publish(topic, message);
}

MQTTBroker::MQTTBroker(){}
MQTTBroker::MQTTBroker(shared_ptr<MQTT_NS::server<>> server):
    __server(server)
{
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
            std::cout << "accept" << std::endl;
            size_t positions = __mqtt_sessions.size();
            __mqtt_sessions.push_back(MQTTBrokerSession(spep, 
                                                        std::make_shared<set<con_sp_t>>(__connections), 
                                                        std::make_shared<mi_sub_con>(__subs)));
            try{
                __mqtt_sessions.at(positions).init(__sp_db_map_login_password);
                //__kill_timer->expires_from_now(boost::posix_time::seconds(__KILL_TIME_CHECK));
                //__kill_timer->async_wait(boost::bind(&MQTTBroker::checkActiveSessions, shared_from_this(),_1));
            }catch(exception &e){
                std::cout <<std::endl<< "ERROR IN POS" << std::endl;
            }
        }
    );
}
/*
void MQTTBroker::checkActiveSessions(const boost::system::error_code& error){
    bool kill = false;
    if(error){
        cerr << "checkActiveSessions: " << error.what() << endl;
        //__kill_timer->expires_from_now(boost::posix_time::seconds(__KILL_TIME_CHECK));
        //__kill_timer->async_wait(boost::bind(&MQTTBroker::checkActiveSessions, shared_from_this(),_1));
        return;
    }
    for(auto i = __mqtt_sessions.begin(), end = __mqtt_sessions.end(); i != end; i++){
        if(!i->isLive()){
            __mqtt_sessions.erase(i);
            kill = true;
            break;
        }
    }
    if(kill){
        __kill_timer->expires_from_now(boost::posix_time::seconds(1));
        __kill_timer->async_wait(boost::bind(&MQTTBroker::checkActiveSessions, shared_from_this(),_1));
    }else{
        __kill_timer->expires_from_now(boost::posix_time::seconds(__KILL_TIME_CHECK));
        __kill_timer->async_wait(boost::bind(&MQTTBroker::checkActiveSessions, shared_from_this(),_1));
    }
    

}
*/

void MQTTBroker::start(shared_ptr<shared_ptr<map<string, string>>> sp_db_map_login_password){
    __sp_db_map_login_password = sp_db_map_login_password;
    //__kill_timer = make_shared<boost::asio::deadline_timer>(*ctx);
    
    /*for (auto i = (*__sp_db_map_login_password)->begin(); i != (*__sp_db_map_login_password)->end(); i++) {
        cout << i->first << " " << i->second << endl;
    }*/
    __server->listen();
}

bool MQTTBroker::publish(string lu_descriptor, string topik, string message){
    for(auto i = __mqtt_sessions.begin(), end = __mqtt_sessions.end(); i != end; i++){
        if((*i).getName() == lu_descriptor){
            (*i).publish(topik, message);
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