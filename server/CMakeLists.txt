/********************************************************************************
* Copyright (c) 2025 Contributors to the Eclipse Foundation
*
* See the NOTICE file(s) distributed with this work for additional
* information regarding copyright ownership.
*
* This program and the accompanying materials are made available under the
* terms of the Apache License Version 2.0 which is available at
* https://www.apache.org/licenses/LICENSE-2.0
*
* SPDX-License-Identifier: Apache-2.0
********************************************************************************/

cmake_minimum_required(VERSION 3.20)
project(rpc VERSION 0.1.0 LANGUAGES CXX)

find_package(spdlog REQUIRED)
find_package(up-cpp REQUIRED)
find_package(up-core-api REQUIRED)
find_package(up-transport-socket-cpp REQUIRED)
find_package(zenohcpp REQUIRED)
find_package(zenohc REQUIRED)
find_package(up-transport-zenoh-cpp REQUIRED)
find_package(protobuf REQUIRED)


set(zenohcpp_INCLUDE_DIR "${zenohcpp_PACKAGE_FOLDER_DEBUG}/include")
set(zenohc_INCLUDE_DIR "${zenohc_PACKAGE_FOLDER_DEBUG}/include")


set(BASE_INCLUDE_DIR /home/user/projects/eclipse-uprotocol)

add_definitions(-DSPDLOG_FMT_EXTERNAL)

# rpc client zenoh utransport
add_executable(rpc_client_z src/main_rpc_client_z.cpp src/can_message.pb.cc src/translator-protobuf.pb.cc)
target_link_libraries(rpc_client_z
    PUBLIC
    	spdlog::spdlog
        up-cpp::up-cpp
        up-core-api::up-core-api
        up-transport-zenoh-cpp::up-transport-zenoh-cpp
        zenohcpp::lib
        protobuf::libprotobuf
        )
target_include_directories(rpc_client_z
    PUBLIC
        ${BASE_INCLUDE_DIR}/up-transport-zenoh-cpp/include/up-transport-zenoh-cpp
        ${zenohc_INCLUDE_DIR}
        ${zenohcpp_INCLUDE_DIR}
        ${protobuf_INCLUDE_DIR}
    PRIVATE
        include
        )

# rpc server zenoh utransport
add_executable(rpc_server_z src/main_rpc_server_z.cpp src/can_message.pb.cc src/translator-protobuf.pb.cc)
target_link_libraries(rpc_server_z
    PUBLIC
    	spdlog::spdlog
        up-cpp::up-cpp
        up-core-api::up-core-api
        up-transport-zenoh-cpp::up-transport-zenoh-cpp
        zenohcpp::lib
        )
target_include_directories(rpc_server_z
    PUBLIC
        ${BASE_INCLUDE_DIR}/up-transport-zenoh-cpp/include/up-transport-zenoh-cpp
        ${zenohc_INCLUDE_DIR}
        ${zenohcpp_INCLUDE_DIR}
    PRIVATE
        include
        )
