cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")

add_library(simple_player SHARED
            simple_player.cpp)

# Include libraries needed for native-audio-jni lib
target_link_libraries(simple_player
                      android
                      log
                      OpenSLES)