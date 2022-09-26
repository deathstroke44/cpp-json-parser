cmake_minimum_required(VERSION 3.23)
project(json_streamer_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(json_streamer_cpp main.cpp
        src/library/json-streaming.hh
        src/library/Dispatcher.hh
        src/library/Event.hh
        src/json-path-query-demo.cpp
        src/library/Custom-data-types.hh)
