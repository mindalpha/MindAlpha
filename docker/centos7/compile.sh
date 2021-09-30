#!/bin/bash

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

set -e
pushd $(dirname ${BASH_SOURCE[0]})/../..
rm -rf build built
path=/usr/local/gcc-7.3.0/bin:${PATH}
path=/usr/local/cmake-3.20.3/bin:${path}
path=/usr/local/ninja-1.10.2/bin:${path}
path=/usr/local/thrift-0.14.1/bin:${path}
prefix="/usr/local/spdlog-1.8.5"
prefix="${prefix};/usr/local/pybind11-2.6.2"
prefix="${prefix};/usr/local/aws-sdk-cpp-1.7.108"
prefix="${prefix};/usr/local/boost-1.76.0"
prefix="${prefix};/usr/local/thrift-0.14.1"
prefix="${prefix};/usr/local/zeromq-4.3.4"
env PATH=${path}                                           \
    PKG_CONFIG_PATH=/usr/local/json11-1.0.0/lib/pkgconfig  \
    LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64             \
    /usr/local/cmake-3.20.3/bin/cmake                      \
    -Wno-dev                                               \
    -G Ninja                                               \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo                      \
    -DCMAKE_INSTALL_PREFIX=built                           \
    -DBUILD_SHARED_LIBS=OFF                                \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON                   \
    -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++      \
    -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja \
    -DCMAKE_PREFIX_PATH="${prefix}"                        \
    -DCMAKE_CXX_FLAGS=-I/usr/local/dbg-macro-0.4.0/include \
    -DPython_ROOT_DIR=/usr/local/python-3.7.7              \
    -H.                                                    \
    -Bbuild
env PATH=${path}                                           \
    PKG_CONFIG_PATH=/usr/local/json11-1.0.0/lib/pkgconfig  \
    LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64             \
    /usr/local/cmake-3.20.3/bin/cmake                      \
    --build build
popd
echo OK
