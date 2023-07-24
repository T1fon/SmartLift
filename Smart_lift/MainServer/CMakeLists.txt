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
	set(GLOBAL_MODULES "../GlobalModules")
endif()

set(Boost_NO_SYSTEM_PATHS		 ON)
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS      OFF)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_USE_DYNAMIC_LIBS        OFF)

set(OPENSSL_NO_SYSTEM_PATHS		 TRUE)
set(OPENSSL_USE_STATIC_LIBS		 TRUE)
set(OPENSSL_MSVC_STATIC_RT		 ON)

set(MOD_JSON_Formatter "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.hpp" "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.cpp")
set(MOD_Server "${MODULES}/WorkerServer/WorkerServer.hpp" "${MODULES}/WorkerServer/WorkerServer.cpp")
set(MOD_Sertificate "${MODULES}/SSLSertificateLocalhost/Sertificate.hpp")
set(MOD_HTTPS_Server "${MODULES}/HTTPSServer/HTTPSServer.hpp" "${MODULES}/HTTPSServer/HTTPSServer.cpp")
set(SOURCE_MODULES ${MOD_HTTPS_Server} ${MOD_Sertificate} ${MOD_Server} ${MOD_JSON_Formatter})

add_executable (MainServer "main.cpp" "MainServer.cpp" "MainServer.hpp" ${SOURCE_MODULES})

find_package(Boost 1.82.0 REQUIRED)
find_package(OpenSSL 3.1.1 REQUIRED)

if(Boost_FOUND AND OpenSSL_FOUND)
	message(CONFIG " OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
	message(CONFIG " OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
	message(CONFIG " OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}")
	message(CONFIG " BOOST_INCLUDEDIR ${Boost_INCLUDE_DIRS}")
	message(CONFIG " MQTT_ROOT ${MQTT_PATH}")
	set_property(TARGET MainServer PROPERTY CXX_STANDARD 20)

	target_include_directories(MainServer PUBLIC ${Boost_INCLUDE_DIRS})
	target_include_directories(MainServer PUBLIC ${OPENSSL_INCLUDE_DIR})
	target_include_directories(MainServer PUBLIC ${MQTT_PATH})
	
	find_package(Boost 1.82.0 COMPONENTS thread chrono atomic REQUIRED)
	find_package(Boost 1.82.0 COMPONENTS json REQUIRED)
	find_package(OPENSSL COMPONENTS crypto ssl REQUIRED)

	target_link_libraries(MainServer PUBLIC Boost::thread)
	target_link_libraries(MainServer PUBLIC Boost::json)
	target_link_libraries(MainServer PUBLIC OpenSSL::SSL)
	target_link_libraries(MainServer PUBLIC OpenSSL::Crypto)
	
else()
	message(CONFIG " Не установлена библиотека Boost версии 1.82.0")
	message(CONFIG " Или не установлена библиотека OpenSSL 3.1.1")
endif()