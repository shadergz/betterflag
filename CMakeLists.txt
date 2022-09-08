cmake_minimum_required(VERSION 3.20)
project(betterflag)

set(CMAKE_CXX_STANDARD 17)

find_package(fmt)

add_executable(betterflag main.cpp betterflag.h)

target_link_libraries(betterflag fmt::fmt)
