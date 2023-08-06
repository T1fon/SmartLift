#pragma once
#include <iostream>
#include <iomanip>
#include <set>
#include <vector>
#include <string>

#include <mqtt_server_cpp.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/asio.hpp>

using namespace std;
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



	class MQTTBroker {
	private:
        con_sp_t __worker;
        shared_ptr<MQTT_NS::server<>> __server;
        string __port;
        //shared_ptr<boost::asio::io_context> __io_ctx;
        boost::asio::io_context* __io_ctx;
        set<con_sp_t> __connections;
        mi_sub_con __subs;
        std::weak_ptr<con_t> __wp;
        using packet_id_t = typename std::remove_reference_t<decltype(*__worker)>::packet_id_t;

        inline void __closeProc(std::set<con_sp_t>& cons, mi_sub_con& subs, con_sp_t const& con);
        
	public:
        MQTTBroker();
		MQTTBroker(shared_ptr<MQTT_NS::server<>> server);
        ~MQTTBroker();
        void setServer(shared_ptr<MQTT_NS::server<>> server);
        void init();
        void start();
        void stop();
        con_sp_t getWorker();
	};
}
