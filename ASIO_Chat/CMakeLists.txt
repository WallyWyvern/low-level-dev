cmake_minimum_required(VERSION 3.14)
project(asio_chat_barebones CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dependencies)
add_subdirectory(src)
