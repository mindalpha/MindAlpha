#
# Copyright 2021 Mobvista
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(mindalpha VERSION 2.0.0.0 LANGUAGES CXX)

find_package(Git REQUIRED)
find_package(Python REQUIRED COMPONENTS Interpreter Development)
find_package(Boost REQUIRED COMPONENTS)
find_package(PkgConfig REQUIRED)

find_package(spdlog REQUIRED CONFIG)
find_package(pybind11 REQUIRED CONFIG)
find_package(AWSSDK REQUIRED CONFIG COMPONENTS s3)

find_package(json11 CONFIG)
if(NOT TARGET json11_static)
    pkg_search_module(JSON11 REQUIRED IMPORTED_TARGET GLOBAL json11)
    add_library(json11_static ALIAS PkgConfig::JSON11)
endif()

find_package(Thrift CONFIG)
if(NOT TARGET thrift::thrift)
    pkg_search_module(THRIFT REQUIRED IMPORTED_TARGET GLOBAL thrift)
    add_library(thrift::thrift ALIAS PkgConfig::THRIFT)
endif()

find_package(ZeroMQ CONFIG)
if(NOT TARGET libzmq-static)
    find_library(ZMQ_LIB zmq)
    if("${ZMQ_LIB}" STREQUAL "ZMQ_LIB-NOTFOUND")
        message(FATAL_ERROR "libzmq not found")
    endif()
    find_path(ZMQ_HEADER zmq.h)
    if("${ZMQ_HEADER}" STREQUAL "ZMQ_HEADER-NOTFOUND")
        message(FATAL_ERROR "zmq.h not found")
    endif()
    add_library(zmq::libzmq STATIC IMPORTED GLOBAL)
    set_target_properties(zmq::libzmq PROPERTIES
        IMPORTED_LOCATION "${ZMQ_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZMQ_HEADER}")
else()
    add_library(zmq::libzmq ALIAS libzmq-static)
endif()

include(cmake/get_project_version.cmake)
include(cmake/get_python_wheel_tag.cmake)
include(cmake/mindalpha_shared.cmake)
include(cmake/python_wheel.cmake)
