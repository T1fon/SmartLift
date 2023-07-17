if(UNIX)
	set(BOOST_PATH "")
else() 
	set(BOOST_ROOT "../../../boost_1_82_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/x64/lib")

	set(OPENSSL_ROOT_DIR "C:/Vato/My_Programs/LKDS/OpenSSLBuild")
	set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")

	set(MQTT_PATH "C:/Vato/My_Programs/LKDS/MQTT_cpp/source/include")
	set(MODULES "Modules")
	set(GLOBAL_MODULES "C:/Vato/My_Programs/LKDS/Smart_Lift/Smart_lift/GlobalModules")
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

set(MOD_JSON_Formatter "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.hpp" "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.cpp")
set(MOD_MSWorker "${MODULES}/MSWorker/MSWorker.hpp" "${MODULES}/MSWorker/MSWorker.cpp")
set(MOD_MQTT_Broker "${MODULES}/MQTTBroker/MQTTBroker.hpp" "${MODULES}/MQTTBroker/MQTTBroker.cpp")
set(SOURCE_MODULES ${MOD_MQTT_Broker} ${MOD_MSWorker} ${MOD_JSON_Formatter})

add_executable (MQTT_Worker "main.cpp" "MQTTWorker.cpp" "MQTTWorker.hpp" ${SOURCE_MODULES})

find_package(Boost 1.82.0 REQUIRED)
find_package(OpenSSL 3.1.1 REQUIRED)

if(Boost_FOUND AND OpenSSL_FOUND)
	message(CONFIG " OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
	message(CONFIG " OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
	message(CONFIG " OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}")
	message(CONFIG " BOOST_INCLUDEDIR ${Boost_INCLUDE_DIRS}")
	message(CONFIG " MQTT_ROOT ${MQTT_PATH}")
	set_property(TARGET MQTT_Worker PROPERTY CXX_STANDARD 20)

	target_include_directories(MQTT_Worker PUBLIC ${Boost_INCLUDE_DIRS})
	target_include_directories(MQTT_Worker PUBLIC ${OPENSSL_INCLUDE_DIR})
	target_include_directories(MQTT_Worker PUBLIC ${MQTT_PATH})
	
	find_package(Boost 1.82.0 COMPONENTS thread chrono atomic log REQUIRED)
	find_package(Boost 1.82.0 COMPONENTS json REQUIRED)
	find_package(OPENSSL COMPONENTS crypto ssl REQUIRED)

	target_link_libraries(MQTT_Worker PUBLIC Boost::thread)
	target_link_libraries(MQTT_Worker PUBLIC Boost::json)
	target_link_libraries(MQTT_Worker PUBLIC Boost::log)
	
	target_link_libraries(MQTT_Worker PUBLIC OpenSSL::SSL)
	target_link_libraries(MQTT_Worker PUBLIC OpenSSL::Crypto)
	
	add_library(mqtt_cpp_iface INTERFACE)
	
else()
	message(CONFIG " Не установлена библиотека Boost версии 1.82.0")
	message(CONFIG " Или не установлена библиотека OpenSSL 3.1.1")
endif()