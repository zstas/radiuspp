cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")

# set the project name
project(radius++)

file(GLOB SOURCES src/*.cpp)

add_library(radiuspp_static STATIC ${SOURCES})
add_library(radiuspp SHARED ${SOURCES})

target_link_libraries(radiuspp PUBLIC boost_system)
target_link_libraries(radiuspp PUBLIC boost_random)
target_link_libraries(radiuspp PUBLIC pthread)

install(TARGETS radiuspp_static DESTINATION lib)
install(TARGETS radiuspp DESTINATION lib)
install(FILES src/radiuspp.hpp DESTINATION include)

add_executable(example example/main.cpp)
include_directories(src/)
target_link_libraries( example radiuspp)