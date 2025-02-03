cmake_minimum_required(VERSION 3.5)

project(sample_ipc LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/common_lib/umsg_proto)
add_subdirectory(${CMAKE_SOURCE_DIR}/common_lib/shared_comm)

set(INCL ${CMAKE_SOURCE_DIR}/common_include ${CMAKE_SOURCE_DIR}/common_lib/shared_comm/include ${CMAKE_BINARY_DIR}/common_lib)
set(LIBS shared_comm umsg_proto)
set(DEPENDS shared_comm umsg_proto)
add_subdirectory(${CMAKE_SOURCE_DIR}/app/server)
add_subdirectory(${CMAKE_SOURCE_DIR}/app/client)
