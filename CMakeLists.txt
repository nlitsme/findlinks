cmake_minimum_required(VERSION 3.10)
project(findlinks)
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_find")

include(boilerplate)

find_package(itslib REQUIRED)
find_package(cpputils REQUIRED)

add_executable(findlinks findlinks.cpp)
target_link_libraries(findlinks itslib cpputils)

