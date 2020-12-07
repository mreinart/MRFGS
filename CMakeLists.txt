cmake_minimum_required(VERSION 2.8)

project(MRFGS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -DCPPHTTPLIB_OPENSSL_SUPPORT ")

set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(/usr/local/include /usr/local/opt/openssl/include )
include_directories(src/fanet_GS src/include src/include/mqtt src/telnet)
link_directories(/usr/local/opt/openssl/lib)

add_executable(MRFGS
        src/fgs_main.cpp
        src/loguru.cpp src/loguru.hpp
        src/telnet/telnet.c src/telnet/telnet.h
        src/Packet.cpp src/Packet.h
        src/Track.cpp src/Track.h
        src/Configuration.cpp src/Configuration.h
        src/Vehicle.cpp src/Vehicle.h
        src/WeatherMeasure.cpp src/WeatherMeasure.h
        src/WeatherStation.cpp src/WeatherStation.h
        src/WeatherStationManager.cpp src/WeatherStationManager.h
        src/FanetWeatherStation.cpp src/FanetWeatherStation.h
        src/HolfuyWeatherStation.cpp src/HolfuyWeatherStation.h
        src/WindyWeatherStation.cpp src/WindyWeatherStation.h
        src/EntityName.cpp src/EntityName.h
        src/GroundStation.cpp src/GroundStation.h
        src/FanetGroundStation.cpp src/FanetGroundStation.h
        src/Landmark.cpp src/Landmark.h
        src/Message.cpp src/Message.h
        src/include/TDequeConcurrent.h
        src/include/rapidjson/rapidjson.h
        src/include/rapidcsv.h
        src/AprsOgnManager.cpp src/AprsOgnManager.h
        src/AprsCwopServer.cpp src/AprsCwopServer.cpp
        src/fanet_GS/fanet_t0_ack.c src/fanet_GS/fanet_t0_ack.h
        src/fanet_GS/fanet_t2_name.c src/fanet_GS/fanet_t2_name.h
        src/fanet_GS/fanet_t1_tracking.c src/fanet_GS/fanet_t1_tracking.h
        src/fanet_GS/fanet_t3_messenger.c src/fanet_GS/fanet_t3_messenger.h
        src/fanet_GS/fanet_t4_service.c src/fanet_GS/fanet_t4_service.h
        src/fanet_GS/fanet_t5_landmark.c src/fanet_GS/fanet_t5_landmark.h
        src/fanet_GS/fanet_t7_tracking.c src/fanet_GS/fanet_t7_tracking.h
        src/fanet_GS/fanet_t8_hwinfo.c src/fanet_GS/fanet_t8_hwinfo.h
        src/fanet_GS/fanet_t9_thermal.c src/fanet_GS/fanet_t9_thermal.h
        src/fanet_GS/fanet_global.c src/fanet_GS/fanet_global.h
        src/fanet_GS/fanet_mac.c src/fanet_GS/fanet_mac.h
        src/fanet_GS/fanet_radio.c src/fanet_GS/fanet_radio.h
        src/fanet_GS/fanet_struct.c src/fanet_GS/fanet_struct.h
        src/fanet_GS/fanet_terminal.c src/fanet_GS/fanet_terminal.h
)

add_executable(inoGS
        src/ino_main.cpp
        src/loguru.cpp src/loguru.hpp
        src/telnet/telnet.c src/telnet/telnet.h
        src/Packet.cpp src/Packet.h
        src/Track.cpp src/Track.h
        src/Configuration.cpp src/Configuration.h
        src/WeatherMeasure.cpp src/WeatherMeasure.h
        src/WeatherStation.cpp src/WeatherStation.h
        src/WeatherStationManager.cpp src/WeatherStationManager.h
        src/HolfuyWeatherStation.cpp src/HolfuyWeatherStation.h
        src/WindyWeatherStation.cpp src/WindyWeatherStation.h
        src/EntityName.cpp src/EntityName.h
        src/FanetWeatherStation.cpp src/FanetWeatherStation.h
        src/GroundStation.cpp src/GroundStation.h
        src/Message.cpp src/Message.h
        src/include/TDequeConcurrent.h
        src/include/rapidjson/rapidjson.h
        src/include/rapidcsv.h
        src/AprsOgnManager.cpp src/AprsOgnManager.h
        src/AprsCwopServer.cpp src/AprsCwopServer.cpp
#        src/fanet_GS/fanet_t0_ack.c src/fanet_GS/fanet_t0_ack.h
        src/fanet_GS/fanet_t2_name.c src/fanet_GS/fanet_t2_name.h
        src/fanet_GS/fanet_t1_tracking.c src/fanet_GS/fanet_t1_tracking.h
        src/fanet_GS/fanet_t3_messenger.c src/fanet_GS/fanet_t3_messenger.h
        src/fanet_GS/fanet_t4_service.c src/fanet_GS/fanet_t4_service.h
        src/fanet_GS/fanet_t5_landmark.c src/fanet_GS/fanet_t5_landmark.h
        src/fanet_GS/fanet_t7_tracking.c src/fanet_GS/fanet_t7_tracking.h
        src/fanet_GS/fanet_t8_hwinfo.c src/fanet_GS/fanet_t8_hwinfo.h
        src/fanet_GS/fanet_t9_thermal.c src/fanet_GS/fanet_t9_thermal.h
        src/fanet_GS/fanet_global.c src/fanet_GS/fanet_global.h
        src/fanet_GS/fanet_mac.c src/fanet_GS/fanet_mac.h
#        src/fanet_GS/fanet_radio.c src/fanet_GS/fanet_radio.h
        src/fanet_GS/fanet_struct.c src/fanet_GS/fanet_struct.h
        src/fanet_GS/fanet_terminal.c src/fanet_GS/fanet_terminal.h
        )

target_link_libraries(MRFGS wiringPi ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} dl rt)
target_link_libraries(inoGS          ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} dl rt)
