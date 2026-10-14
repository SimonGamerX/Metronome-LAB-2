cmake_minimum_required(VERSION 3.10)
project(main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Search for cpprestsdk using standard paths and common Raspberry Pi multiarch paths
find_package(cpprestsdk QUIET)

if(NOT cpprestsdk_FOUND)
    list(APPEND CMAKE_PREFIX_PATH
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/cpprestsdk/
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/
        /usr/lib/aarch64-linux-gnu/cmake/cpprestsdk/
        /usr/lib/arm-linux-gnueabihf/cmake/cpprestsdk/
        /usr/lib/cmake/cpprestsdk/
        /usr/share/cmake/cpprestsdk/
    )
    find_package(cpprestsdk REQUIRED)
endif()

add_executable(main main.cpp metronome.cpp)
target_link_libraries(main PRIVATE cpprestsdk::cpprest wiringPi boost_system)
