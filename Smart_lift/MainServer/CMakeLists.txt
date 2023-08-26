
set(MODULES "Modules")
set(GLOBAL_MODULES "../GlobalModules")


if(UNIX)
	set(BOOST_ROOT "/home/vato/Vato/SmartLift/boost_1_83_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/bin.v2/libs")

	set(OPENSSL_ROOT_DIR "/home/vato/Vato/SmartLift/openssl/static")
	set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
	

	set(OPENSSL_NO_SYSTEM_PATHS		 TRUE)
	set(OPENSSL_USE_STATIC_LIBS		 TRUE)
	set(OPENSSL_MSVC_STATIC_RT		 ON)
	set(ZLIB_INCLUDE_DIRS "/home/vato/Vato/tools/zlib/zlib-1.3")
	set(ZLIB_USE_STATIC_LIBS ON)
	#add_subdirectory("../GlobalModules/Log")
else() 
	set(BOOST_ROOT "../../../boost_1_82_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/x64/lib")

	set(OPENSSL_ROOT_DIR "C:/Vato/My_Programs/LKDS/OpenSSLBuild")
	set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")

	set(MQTT_PATH "C:/Vato/My_Programs/LKDS/MQTT_cpp/source/include")
	set(OPENSSL_NO_SYSTEM_PATHS		 TRUE)
	set(OPENSSL_USE_STATIC_LIBS		 TRUE)
	set(OPENSSL_MSVC_STATIC_RT		 ON)

	
endif()



set(Boost_NO_SYSTEM_PATHS		 ON)
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS      ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_USE_DYNAMIC_LIBS        OFF)



#set(MOD_Net_Repeater "${MODULES}/NetRepeater/NetRepeater.hpp" "${MODULES}/NetRepeater/NetRepeater.cpp")
set(MOD_Config "${GLOBAL_MODULES}/Config/Config.cpp" "${GLOBAL_MODULES}/Config/Config.hpp")
set(MOD_Log "${GLOBAL_MODULES}/Log/Log.cpp" "${GLOBAL_MODULES}/Log/Log.hpp")
set(MOD_JSON_Formatter "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.hpp" "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.cpp")
set(MOD_Server "${MODULES}/WorkerServer/WorkerServer.hpp" "${MODULES}/WorkerServer/WorkerServer.cpp")
set(MOD_Sertificate "${MODULES}/SSLSertificateLocalhost/Sertificate.hpp")
set(MOD_HTTPS_Server "${MODULES}/HTTPSServer/HTTPSServer.hpp" "${MODULES}/HTTPSServer/HTTPSServer.cpp")
set(MOD_Client_DB "${GLOBAL_MODULES}/ClientDB/ClientDB.hpp")

set(SOURCE_MODULES ${MOD_HTTPS_Server} ${MOD_Sertificate} ${MOD_Server} ${MOD_JSON_Formatter} ${MOD_Log} ${MOD_Config} ${MOD_Client_DB})

add_executable (MainServer "MainServer.hpp" "MainServer.cpp" ${SOURCE_MODULES})

find_package(Boost REQUIRED) 
find_package(OpenSSL REQUIRED)

if(Boost_FOUND AND OpenSSL_FOUND)

	set_property(TARGET MainServer PROPERTY CXX_STANDARD 20)

	target_include_directories(MainServer PUBLIC ${Boost_INCLUDE_DIRS})
	target_include_directories(MainServer PUBLIC ${OPENSSL_INCLUDE_DIR})
	target_include_directories(MainServer PUBLIC ${MQTT_PATH})
	if(UNIX)
		target_include_directories(MainServer PUBLIC ${ZLIB_INCLUDE_DIRS})
	endif()

	
	find_package(Boost COMPONENTS thread chrono atomic locale REQUIRED)
	find_package(Boost COMPONENTS json REQUIRED)
	find_package(OpenSSL COMPONENTS Crypto REQUIRED)
	find_package(OpenSSL COMPONENTS SSL REQUIRED)
	find_package(ZLIB COMPONENTS ZLIB REQUIRED)
	
	target_link_libraries(MainServer PUBLIC Boost::thread)
	target_link_libraries(MainServer PUBLIC Boost::json)
	target_link_libraries(MainServer PUBLIC Boost::locale)
	target_link_libraries(MainServer PUBLIC OpenSSL::SSL)
	target_link_libraries(MainServer PUBLIC OpenSSL::Crypto)
	if(UNIX)
		target_link_libraries(MainServer PUBLIC ZLIB::ZLIB)
	endif()
	
else()
	message(CONFIG " �� ����������� ���������� Boost ������ 1.82.0")
	message(CONFIG " ��� �� ����������� ���������� OpenSSL 3.1.1")
endif()