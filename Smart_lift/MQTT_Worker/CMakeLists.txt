set(MODULES "Modules")
set(GLOBAL_MODULES "../GlobalModules")

if(UNIX)
	set(BOOST_ROOT "/home/vato/Vato/SmartLift/boost_1_83_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/bin.v2/libs")

	set(MQTT_PATH "/home/vato/Vato/SmartLift/mqtt/mqtt_cpp/include")
else() 
	set(BOOST_ROOT "../../../boost_1_82_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/x64/lib")

	set(MQTT_PATH "C:/Vato/My_Programs/LKDS/MQTT_cpp/source/include")
	set(CMAKE_CXX_FLAGS "/bigobj /EHsc")
endif()



set(Boost_NO_SYSTEM_PATHS		 ON)
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS      OFF)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_USE_DYNAMIC_LIBS        ON)

set(OPENSSL_NO_SYSTEM_PATHS		 TRUE)
set(OPENSSL_USE_STATIC_LIBS		 TRUE)
set(OPENSSL_MSVC_STATIC_RT		 ON)

set(MOD_CONFIG "${GLOBAL_MODULES}/Config/Config.cpp" "${GLOBAL_MODULES}/Config/Config.hpp")
set(MOD_LOG "${GLOBAL_MODULES}/Log/Log.cpp" "${GLOBAL_MODULES}/Log/Log.hpp")
set(MOD_JSON_Formatter "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.hpp" "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.cpp")
set(MOD_MSWorker "${MODULES}/MSWorker/MSWorker.hpp" "${MODULES}/MSWorker/MSWorker.cpp")
set(MOD_MQTT_Broker "${MODULES}/MQTTBroker/MQTTBroker.hpp" "${MODULES}/MQTTBroker/MQTTBroker.cpp")
set(MOD_Client_DB "${GLOBAL_MODULES}/ClientDB/ClientDB.hpp")
set(SOURCE_MODULES ${MOD_MQTT_Broker} ${MOD_MSWorker} ${MOD_JSON_Formatter} ${MOD_LOG} ${MOD_CONFIG} ${MOD_Client_DB})

add_executable (MQTT_Worker "MQTTWorker.cpp" "MQTTWorker.hpp" ${SOURCE_MODULES})

find_package(Boost REQUIRED)

if(Boost_FOUND)
	message(CONFIG " OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
	message(CONFIG " OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
	message(CONFIG " OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}")
	message(CONFIG " BOOST_INCLUDEDIR ${Boost_INCLUDE_DIRS}")
	message(CONFIG " MQTT_ROOT ${MQTT_PATH}")
	set_property(TARGET MQTT_Worker PROPERTY CXX_STANDARD 20)

	target_include_directories(MQTT_Worker PUBLIC ${Boost_INCLUDE_DIRS})
	target_include_directories(MQTT_Worker PUBLIC ${MQTT_PATH})
	
	find_package(Boost COMPONENTS thread chrono atomic log REQUIRED)
	find_package(Boost COMPONENTS json REQUIRED)

	target_link_libraries(MQTT_Worker PUBLIC Boost::thread)
	target_link_libraries(MQTT_Worker PUBLIC Boost::json)
	target_link_libraries(MQTT_Worker PUBLIC Boost::log)
	
	add_library(mqtt_cpp_iface INTERFACE)

	target_include_directories(MQTT_Worker PUBLIC ${GLOBAL_MODULES})
	target_include_directories(MQTT_Worker PUBLIC ${MODULES})
else()
	message(CONFIG " �� ����������� ���������� Boost ������ 1.82.0")
	message(CONFIG " ��� �� ����������� ���������� OpenSSL 3.1.1")
endif()