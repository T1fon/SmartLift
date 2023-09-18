# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.8)

#project ("Smart_lift")

# Включите подпроекты.
#add_subdirectory ("Smart_lift")

project("MainServer")
add_subdirectory("Smart_lift/MainServer")

project("MQTT_Worker")
add_subdirectory("Smart_lift/MQTT_Worker")

#project ("Worker")
#add_subdirectory("Smart_lift/Worker")

#project("DataBase")
#add_subdirectory("Smart_lift/DataBase")

#project("Resurrector")
#add_subdirectory("Smart_lift/Resurrector")
