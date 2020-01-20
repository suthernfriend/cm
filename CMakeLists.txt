cmake_minimum_required(VERSION 3.16)
project(cm)

set(EXECUTABLE_NAME cm)
set(CMAKE_CXX_STANDARD 17)
set(Boost_USE_STATIC_LIBS on)

find_package(yaml-cpp REQUIRED)
find_package(Boost 1.71 REQUIRED filesystem date_time)
find_library(YAML_CPP_STATIC_LIB libyaml-cpp.a)

include_directories(
        ${Boost_INCLUDE_DIR}
        ${YAML_CPP_INCLUDE_DIR}
        )

add_executable(${EXECUTABLE_NAME} main.cpp application.cpp application.h line_buffer.h child.cpp child.h config_map.cpp config_map.h constants.h logger.cpp logger.h)

target_link_libraries(${EXECUTABLE_NAME}
        ${YAML_CPP_STATIC_LIB}
        ${Boost_LIBRARIES}
        pthread
        "-static"
        )
