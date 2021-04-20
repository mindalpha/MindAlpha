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
find_package(Python REQUIRED COMPONENTS Interpreter)

find_package(spdlog REQUIRED CONFIG)
find_package(pybind11 REQUIRED CONFIG)
find_package(json11 REQUIRED CONFIG)
find_package(AWSSDK REQUIRED CONFIG COMPONENTS s3)

include(cmake/get_project_version.cmake)
include(cmake/get_python_wheel_tag.cmake)
include(cmake/mindalpha_shared.cmake)
include(cmake/python_wheel.cmake)
