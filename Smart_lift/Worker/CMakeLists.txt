# CMakeList.txt: проект CMake для Smart_lift; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.

if(UNIX)
	set(BOOST_PATH "")
else() 
	set(BOOST_ROOT "../../../boost_1_82_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/x64/lib")

	set(OPENSSL_ROOT_DIR "C:/Users/Danil Ignatev/source/repos/VatoGoL/OpenSSLBuild")
	set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")


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

#set(MOD_Client "${MODULES}/ClientMtW/newClient.cpp" "${MODULES}/ClientMtW/newClient.hpp")
set(MOD_JSON "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.cpp" "${GLOBAL_MODULES}/JSONFormatter/JSONFormatter.hpp")
#set(SOURCE_MODULES ${MOD_Client})
add_executable (Vorker "Worker.cpp" "Worker.hpp" ${MOD_JSON})

find_package(Boost 1.82.0 REQUIRED)
find_package(OpenSSL 3.1.1 REQUIRED)


if(Boost_FOUND AND OpenSSL_FOUND)
	message(CONFIG " OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
	message(CONFIG " OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
	message(CONFIG " OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}")
	message(CONFIG " BOOST_INCLUDEDIR ${Boost_INCLUDE_DIRS}")


	set_property(TARGET Vorker PROPERTY CXX_STANDARD 20)

	target_include_directories(Vorker PUBLIC ${Boost_INCLUDE_DIRS})
	target_include_directories(Vorker PUBLIC ${OPENSSL_INCLUDE_DIR})
	
	find_package(Boost 1.82.0 COMPONENTS thread chrono atomic REQUIRED)
	find_package(OPENSSL COMPONENTS crypto ssl REQUIRED)


	target_link_libraries(Vorker PUBLIC Boost::thread)
	target_link_libraries(Vorker PUBLIC OpenSSL::SSL)
	target_link_libraries(Vorker PUBLIC OpenSSL::Crypto)
else()
	message(CONFIG " Не установлена библиотека Boost версии 1.82.0")
	message(CONFIG " Или не установлена библиотека OpenSSL 3.1.1")
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
