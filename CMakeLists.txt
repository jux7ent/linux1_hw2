cmake_minimum_required(VERSION 3.10)
project(miniFS C)

set(CMAKE_C_STANDARD 99)

include_directories(network)

add_executable(serverFS network/server.c)

add_executable(clientFS network/client.c)
