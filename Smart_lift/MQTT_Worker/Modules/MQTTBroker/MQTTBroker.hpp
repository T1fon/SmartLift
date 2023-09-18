#pragma once
#include <iostream>
#include <iomanip>
#include <set>
#include <vector>
#include <string>
#include <map>
#include <mqtt_server_cpp.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
namespace mqtt_broker {
	namespace mi = boost::multi_index;
    using con_t = MQTT_NS::server<>::endpoint_t;
    using con_sp_t = std::shared_ptr<con_t>;

    struct sub_con {
        sub_con(MQTT_NS::buffer topic, con_sp_t con, MQTT_NS::qos qos_value)
            :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value) {}
        MQTT_NS::buffer topic;
        con_sp_t con;
        MQTT_NS::qos qos_value;
    };

    struct tag_topic {};
    struct tag_con {};
    struct tag_con_topic {};

    using mi_sub_con = mi::multi_index_container<
        sub_con,
        mi::indexed_by<
        mi::ordered_unique<
        mi::tag<tag_con_topic>,
        mi::composite_key<
        sub_con,
        BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con),
        BOOST_MULTI_INDEX_MEMBER(sub_con, MQTT_NS::buffer, topic)
        >
        >,
        mi::ordered_non_unique<
        mi::tag<tag_topic>,
        BOOST_MULTI_INDEX_MEMBER(sub_con, MQTT_NS::buffer, topic)
        >,
        mi::ordered_non_unique<
        mi::tag<tag_con>,
        BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con)
        >
        >
    >;

    class MQTTBrokerSession{
        private:
            con_t& __end_point;
            std::weak_ptr<con_t> __wp;
            shared_ptr<set<con_sp_t>> __connections;
            shared_ptr<mi_sub_con> __subs;
            con_sp_t __worker;
            con_sp_t __move_worker;
            string __name;
            shared_ptr<shared_ptr<map<string, string>>> __sp_db_map_login_password;
            
            bool __is_live;
            inline void __closeProc(std::set<con_sp_t>& cons, mi_sub_con& subs, con_sp_t const& con);
        public:
            MQTTBrokerSession(con_sp_t end_point, shared_ptr<set<con_sp_t>> connections, shared_ptr<mi_sub_con> subs);
            ~MQTTBrokerSession();

            void init(shared_ptr<shared_ptr<map<string, string>>> sp_db_map_login_password);
            void setName(string name);
            string getName();
            void publish(string topic,string message);
            bool isLive();
    };

	class MQTTBroker: public std::enable_shared_from_this<MQTTBroker> {
	private:
        shared_ptr<MQTT_NS::server<>> __server;
        string __port;
        set<con_sp_t> __connections;
        mi_sub_con __subs;
        //static const int __KILL_TIME_CHECK = 3; 
        //shared_ptr<boost::asio::deadline_timer> __kill_timer;
        shared_ptr<shared_ptr<map<string, string>>> __sp_db_map_login_password;
        vector<MQTTBrokerSession> __mqtt_sessions;

        void checkActiveSessions(const boost::system::error_code& error);
	public:
		MQTTBroker();
        MQTTBroker(shared_ptr<MQTT_NS::server<>> server);
        ~MQTTBroker();
        void setServer(shared_ptr<MQTT_NS::server<>> server);
        void init();
        void start(shared_ptr<shared_ptr<map<string, string>>> sp_db_map_login_password);
        void stop();
        bool publish(string lu_description, string topik, string message);
	};
}
