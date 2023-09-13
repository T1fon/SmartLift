# CMakeList.txt: ������ CMake ��� Smart_lift; �������� �������� ��� � �����������,
# ������� ����� ������ ��� ����������� �������.
#

# �������� �������� � ����������� ���� ����� �������.

if(UNIX)
	set(BOOST_ROOT "/home/tifon/boost_1_83_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/bin.v2/libs")
else() 
	set(BOOST_ROOT "../../../boost_1_82_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/bin/x64/lib")
endif()

set(GLOBAL_MODULES "../GlobalModules")

set(Boost_NO_SYSTEM_PATHS		 ON)
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS      OFF)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_USE_DYNAMIC_LIBS        OFF)


set(MOD_CONFIG "${GLOBAL_MODULES}/Config/Config.cpp" "${GLOBAL_MODULES}/Config/Config.hpp")
set(MOD_LOG "${GLOBAL_MODULES}/Log/Log.cpp" "${GLOBAL_MODULES}/Log/Log.hpp")
set(SOURCE_MODULES ${MOD_CONFIG} ${MOD_LOG})
add_executable (Resurrector "Resurrector.cpp" "Resurrector.hpp" ${SOURCE_MODULES})

find_package(Boost REQUIRED)

if(Boost_FOUND)

	set_property(TARGET Resurrector PROPERTY CXX_STANDARD 20)

	target_include_directories(Resurrector PUBLIC ${Boost_INCLUDE_DIRS})
	
	find_package(Boost COMPONENTS thread chrono atomic REQUIRED)


	target_link_libraries(Resurrector PUBLIC Boost::thread)

else()
	message(CONFIG " �� ����������� ���������� Boost ������ 1.82.0")
	message(CONFIG " ��� �� ����������� ���������� OpenSSL 3.1.1")
endif()

# TODO: �������� ����� � ������� �������, ���� ��� ����������.

