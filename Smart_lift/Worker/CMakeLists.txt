
if(UNIX)
	set(BOOST_ROOT "/home/tifon/boost_1_83_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/bin.v2/libs")
else() 
	set(BOOST_ROOT "../../../boost_1_82_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/x64/lib")
endif()

set(MODULES "Modules")
set(GLOBAL_MODULES "../GlobalModules")
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

set(MOD_JSON "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.cpp" "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.hpp")
set(MOD_CONFIG "${GLOBAL_MODULES}/Config/Config.cpp" "${GLOBAL_MODULES}/Config/Config.hpp")
set(MOD_LOG "${GLOBAL_MODULES}/Log/Log.cpp" "${GLOBAL_MODULES}/Log/Log.hpp")
set(MOD_SERVER "${MODULES}/Worker/Worker.hpp")
set(MOD_CLIENT "${GLOBAL_MODULES}/ClientDB/ClientDB.hpp")
set(SOURCE_MODULES ${MOD_JSON} ${MOD_CONFIG} ${MOD_LOG} ${MOD_SERVER} ${MOD_CLIENT})


add_executable (Worker "main.cpp" "WorkerServer.hpp" ${SOURCE_MODULES})

find_package(Boost REQUIRED)

if(Boost_FOUND)
	message(CONFIG " OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
	message(CONFIG " OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
	message(CONFIG " OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}")
	message(CONFIG " BOOST_INCLUDEDIR ${Boost_INCLUDE_DIRS}")


	set_property(TARGET Worker PROPERTY CXX_STANDARD 20)

	target_include_directories(Worker PUBLIC ${Boost_INCLUDE_DIRS})
	
	find_package(Boost COMPONENTS thread chrono atomic locale REQUIRED)
	find_package(Boost COMPONENTS json REQUIRED)

	target_link_libraries(Worker PUBLIC Boost::thread)
	target_link_libraries(Worker PUBLIC Boost::locale)
	target_link_libraries(Worker PUBLIC Boost::json)
else()
	message(CONFIG " �� ����������� ���������� Boost ������ 1.82.0")
	message(CONFIG " ��� �� ����������� ���������� OpenSSL 3.1.1")
endif()

# TODO: �������� ����� � ������� �������, ���� ��� ����������.
