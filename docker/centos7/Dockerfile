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

FROM centos:7

ENV LANG=C
RUN ln -svf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
RUN yum update -y && yum install -y awscli which sudo

RUN pushd /tmp &&                                                        \
    curl -L -O https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz && \
    tar -xf gcc-7.3.0.tar.xz &&                                          \
    cd gcc-7.3.0 &&                                                      \
    yum install -y wget bzip2 &&                                         \
    ./contrib/download_prerequisites &&                                  \
    cd .. &&                                                             \
    mkdir gcc-7.3.0-build &&                                             \
    cd gcc-7.3.0-build &&                                                \
    yum install -y gcc gcc-c++ make file &&                              \
    ../gcc-7.3.0/configure                                               \
        --prefix=/usr/local/gcc-7.3.0                                    \
        --enable-languages=c,c++ --disable-multilib &&                   \
    make -j$(nproc) &&                                                   \
    make install &&                                                      \
    cd .. &&                                                             \
    rm -rf gcc-7.3.0-build &&                                            \
    rm -rf gcc-7.3.0 &&                                                  \
    rm -f https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz &&      \
    popd &&                                                              \
    ln -svf gcc-7.3.0 /usr/local/gcc &&                                  \
    yum install -y glibc-static &&                                       \
    echo OK: gcc

RUN curl -L -O https://github.com/Kitware/CMake/releases/download/v3.20.3/cmake-3.20.3-Linux-x86_64.tar.gz && \
    mkdir -p /usr/local/cmake-3.20.3 &&                                                                       \
    tar -xf cmake-3.20.3-Linux-x86_64.tar.gz -C /usr/local/cmake-3.20.3 --strip-components 1 &&               \
    rm -f cmake-3.20.3-Linux-x86_64.tar.gz &&                                                                 \
    ln -svf cmake-3.20.3 /usr/local/cmake &&                                                                  \
    echo OK: cmake

RUN yum install -y bzip2 zip unzip zip-devel &&                                                  \
    curl -L -O https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip && \
    mkdir -p /usr/local/ninja-1.10.2/bin &&                                                      \
    unzip ninja-linux.zip -d /usr/local/ninja-1.10.2/bin &&                                      \
    rm -f ninja-linux.zip &&                                                                     \
    ln -svf ninja-1.10.2 /usr/local/ninja &&                                                     \
    echo OK: ninja

RUN pushd /tmp &&                                                                                           \
    curl -L -O https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz &&      \
    tar -xf boost_1_76_0.tar.gz &&                                                                          \
    mkdir -p boost_1_76_0/cc-wrappers/bin &&                                                                \
    ln -svf /usr/local/gcc-7.3.0/bin/g++ boost_1_76_0/cc-wrappers/bin/g++ &&                                \
    pushd boost_1_76_0/tools/build &&                                                                       \
    src/engine/build.sh --cxx=${PWD}/../../cc-wrappers/bin/g++ --cxxflags=-static gcc &&                    \
    env PATH=${PWD}/../../cc-wrappers/bin:${PATH} src/engine/b2 --prefix=/usr/local/boost-1.76.0 install && \
    popd &&                                                                                                 \
    pushd boost_1_76_0/tools/boostdep/build &&                                                              \
    env PATH=${PWD}/../../../cc-wrappers/bin:${PATH} ../../build/src/engine/b2 toolset=gcc variant=release  \
        debug-symbols=off link=static runtime-link=static threadapi=pthread cxxflags=-fPIC -j$(nproc) &&    \
    cp ../../../dist/bin/boostdep /usr/local/boost-1.76.0/bin &&                                            \
    popd &&                                                                                                 \
    pushd boost_1_76_0 &&                                                                                   \
    env PATH=${PWD}/cc-wrappers/bin:${PATH} tools/build/src/engine/b2 toolset=gcc variant=release           \
        debug-symbols=on link=static runtime-link=shared threadapi=pthread cxxflags=-fPIC -j$(nproc)        \
        --without-mpi --without-graph_parallel --without-python --prefix=/usr/local/boost-1.76.0 install && \
    popd &&                                                                                                 \
    rm -rf boost_1_76_0 &&                                                                                  \
    rm -f boost_1_76_0.tar.gz &&                                                                            \
    popd &&                                                                                                 \
    ln -svf boost-1.76.0 /usr/local/boost &&                                                                \
    echo OK: boost

RUN pushd /tmp &&                                                                               \
    curl -L -O https://github.com/zeromq/libzmq/releases/download/v4.3.4/zeromq-4.3.4.tar.gz && \
    tar -xf zeromq-4.3.4.tar.gz &&                                                              \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                              \
        /usr/local/cmake-3.20.3/bin/cmake                                                       \
        -Wno-dev                                                                                \
        -G Ninja                                                                                \
        -DCMAKE_BUILD_TYPE=Release                                                              \
        -DCMAKE_INSTALL_PREFIX=/usr/local/zeromq-4.3.4                                          \
        -DBUILD_SHARED_LIBS=OFF                                                                 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                    \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                         \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                       \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                  \
        -Hzeromq-4.3.4                                                                          \
        -Bzeromq-4.3.4-build &&                                                                 \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                              \
        /usr/local/cmake-3.20.3/bin/cmake                                                       \
        --build zeromq-4.3.4-build                                                              \
        --config Release --target install &&                                                    \
    rm -rf zeromq-4.3.4-build &&                                                                \
    rm -rf zeromq-4.3.4 &&                                                                      \
    rm -f zeromq-4.3.4.tar.gz &&                                                                \
    popd &&                                                                                     \
    ln -svf zeromq-4.3.4 /usr/local/zeromq &&                                                   \
    echo OK: zeromq

RUN pushd /tmp &&                                                                               \
    curl -L -o fmt-7.1.3.tar.gz https://github.com/fmtlib/fmt/archive/refs/tags/7.1.3.tar.gz && \
    tar -xf fmt-7.1.3.tar.gz &&                                                                 \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                              \
        /usr/local/cmake-3.20.3/bin/cmake                                                       \
        -Wno-dev                                                                                \
        -G Ninja                                                                                \
        -DCMAKE_BUILD_TYPE=Release                                                              \
        -DCMAKE_INSTALL_PREFIX=/usr/local/fmt-7.1.3                                             \
        -DBUILD_SHARED_LIBS=OFF                                                                 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                    \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                         \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                       \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                  \
        -DFMT_TEST=OFF                                                                          \
        -DFMT_DOC=OFF                                                                           \
        -Hfmt-7.1.3                                                                             \
        -Bfmt-7.1.3-build &&                                                                    \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                              \
        /usr/local/cmake-3.20.3/bin/cmake                                                       \
        --build fmt-7.1.3-build                                                                 \
        --config Release --target install &&                                                    \
    rm -rf fmt-7.1.3-build &&                                                                   \
    rm -rf fmt-7.1.3 &&                                                                         \
    rm -f fmt-7.1.3.tar.gz &&                                                                   \
    popd &&                                                                                     \
    ln -svf fmt-7.1.3 /usr/local/fmt &&                                                         \
    echo OK: fmt

RUN pushd /tmp &&                                                                                      \
    curl -L -o spdlog-1.8.5.tar.gz https://github.com/gabime/spdlog/archive/refs/tags/v1.8.5.tar.gz && \
    tar -xf spdlog-1.8.5.tar.gz &&                                                                     \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                     \
        /usr/local/cmake-3.20.3/bin/cmake                                                              \
        -Wno-dev                                                                                       \
        -G Ninja                                                                                       \
        -DCMAKE_BUILD_TYPE=Release                                                                     \
        -DCMAKE_INSTALL_PREFIX=/usr/local/spdlog-1.8.5                                                 \
        -DBUILD_SHARED_LIBS=OFF                                                                        \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                           \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                              \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                         \
        -DCMAKE_PREFIX_PATH=/usr/local/fmt-7.1.3                                                       \
        -DSPDLOG_FMT_EXTERNAL=ON                                                                       \
        -DSPDLOG_BUILD_EXAMPLE=OFF                                                                     \
        -DSPDLOG_BUILD_TESTS=OFF                                                                       \
        -Hspdlog-1.8.5                                                                                 \
        -Bspdlog-1.8.5-build &&                                                                        \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                     \
        /usr/local/cmake-3.20.3/bin/cmake                                                              \
        --build spdlog-1.8.5-build                                                                     \
        --config Release --target install &&                                                           \
    rm -rf spdlog-1.8.5-build &&                                                                       \
    rm -rf spdlog-1.8.5 &&                                                                             \
    rm -f spdlog-1.8.5.tar.gz &&                                                                       \
    popd &&                                                                                            \
    ln -svf spdlog-1.8.5 /usr/local/spdlog &&                                                          \
    echo OK: spdlog

RUN pushd /tmp &&                                                                                       \
    curl -L -o json11-1.0.0.tar.gz https://github.com/dropbox/json11/archive/refs/tags/v1.0.0.tar.gz && \
    tar -xf json11-1.0.0.tar.gz &&                                                                      \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                      \
        /usr/local/cmake-3.20.3/bin/cmake                                                               \
        -Wno-dev                                                                                        \
        -G Ninja                                                                                        \
        -DCMAKE_BUILD_TYPE=Release                                                                      \
        -DCMAKE_INSTALL_PREFIX=/usr/local/json11-1.0.0                                                  \
        -DBUILD_SHARED_LIBS=OFF                                                                         \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                            \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                 \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                               \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                          \
        -Hjson11-1.0.0                                                                                  \
        -Bjson11-1.0.0-build &&                                                                         \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                      \
        /usr/local/cmake-3.20.3/bin/cmake                                                               \
        --build json11-1.0.0-build                                                                      \
        --config Release --target install &&                                                            \
    rm -rf json11-1.0.0-build &&                                                                        \
    rm -rf json11-1.0.0 &&                                                                              \
    rm -f json11-1.0.0.tar.gz &&                                                                        \
    popd &&                                                                                             \
    ln -svf json11-1.0.0 /usr/local/json11 &&                                                           \
    echo OK: json11

RUN pushd /tmp &&                                                                                          \
    curl -L -o pybind11-2.6.2.tar.gz https://github.com/pybind/pybind11/archive/refs/tags/v2.6.2.tar.gz && \
    tar -xf pybind11-2.6.2.tar.gz &&                                                                       \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                         \
        /usr/local/cmake-3.20.3/bin/cmake                                                                  \
        -Wno-dev                                                                                           \
        -G Ninja                                                                                           \
        -DCMAKE_BUILD_TYPE=Release                                                                         \
        -DCMAKE_INSTALL_PREFIX=/usr/local/pybind11-2.6.2                                                   \
        -DBUILD_SHARED_LIBS=OFF                                                                            \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                               \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                    \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                                  \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                             \
        -DPYBIND11_INSTALL=ON                                                                              \
        -DPYBIND11_TEST=OFF                                                                                \
        -Hpybind11-2.6.2                                                                                   \
        -Bpybind11-2.6.2-build &&                                                                          \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                         \
        /usr/local/cmake-3.20.3/bin/cmake                                                                  \
        --build pybind11-2.6.2-build                                                                       \
        --config Release --target install &&                                                               \
    rm -rf pybind11-2.6.2-build &&                                                                         \
    rm -rf pybind11-2.6.2 &&                                                                               \
    rm -f pybind11-2.6.2.tar.gz &&                                                                         \
    popd &&                                                                                                \
    ln -svf pybind11-2.6.2 /usr/local/pybind11 &&                                                          \
    echo OK: pybind11

RUN pushd /tmp &&                                                                                                 \
    curl -L -o openssl-1.1.1.tar.gz https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1k.tar.gz && \
    mkdir -p openssl-1.1.1 &&                                                                                     \
    tar -xf openssl-1.1.1.tar.gz -C openssl-1.1.1 --strip-components 1 &&                                         \
    cd openssl-1.1.1 &&                                                                                           \
    yum install -y perl &&                                                                                        \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                \
        CC=/usr/local/gcc-7.3.0/bin/gcc                                                                           \
        CXX=/usr/local/gcc-7.3.0/bin/g++                                                                          \
        ./config                                                                                                  \
        --prefix=/usr/local/openssl-1.1.1                                                                         \
        -fPIC &&                                                                                                  \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                \
        CC=/usr/local/gcc-7.3.0/bin/gcc                                                                           \
        CXX=/usr/local/gcc-7.3.0/bin/g++                                                                          \
        make                                                                                                      \
        -j$(nproc) &&                                                                                             \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                \
        CC=/usr/local/gcc-7.3.0/bin/gcc                                                                           \
        CXX=/usr/local/gcc-7.3.0/bin/g++                                                                          \
        make                                                                                                      \
        install &&                                                                                                \
    find /usr/local/openssl-1.1.1/lib \( -type f -or -type l \) -name '*.so*' -exec rm {} \; &&                   \
    cd .. &&                                                                                                      \
    rm -rf openssl-1.1.1 &&                                                                                       \
    rm -f openssl-1.1.1.tar.gz &&                                                                                 \
    popd &&                                                                                                       \
    ln -svf openssl-1.1.1 /usr/local/openssl &&                                                                   \
    echo OK: openssl

RUN pushd /tmp &&                                                                                                                                   \
    curl -L -o libevent-2.1.12.tar.gz https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz && \
    mkdir -p libevent-2.1.12 &&                                                                                                                     \
    tar -xf libevent-2.1.12.tar.gz -C libevent-2.1.12 --strip-components 1 &&                                                                       \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                                                  \
        /usr/local/cmake-3.20.3/bin/cmake                                                                                                           \
        -Wno-dev                                                                                                                                    \
        -G Ninja                                                                                                                                    \
        -DCMAKE_BUILD_TYPE=Release                                                                                                                  \
        -DCMAKE_INSTALL_PREFIX=/usr/local/libevent-2.1.12                                                                                           \
        -DBUILD_SHARED_LIBS=OFF                                                                                                                     \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                                                                        \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                                                             \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                                                                           \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                                                                      \
        -DOPENSSL_ROOT_DIR=/usr/local/openssl-1.1.1                                                                                                 \
        -DEVENT__DISABLE_SAMPLES=ON                                                                                                                 \
        -DEVENT__DISABLE_BENCHMARK=ON                                                                                                               \
        -DEVENT__DISABLE_REGRESS=ON                                                                                                                 \
        -DEVENT__DISABLE_TESTS=ON                                                                                                                   \
        -Hlibevent-2.1.12                                                                                                                           \
        -Blibevent-2.1.12-build &&                                                                                                                  \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                                                  \
        /usr/local/cmake-3.20.3/bin/cmake                                                                                                           \
        --build libevent-2.1.12-build                                                                                                               \
        --config Release --target install &&                                                                                                        \
    rm -rf libevent-2.1.12-build &&                                                                                                                 \
    rm -rf libevent-2.1.12 &&                                                                                                                       \
    rm -f libevent-2.1.12.tar.gz &&                                                                                                                 \
    popd &&                                                                                                                                         \
    ln -svf libevent-2.1.12 /usr/local/libevent &&                                                                                                  \
    echo OK: libevent

RUN pushd /tmp &&                                                                                        \
    curl -L -o thrift-0.14.1.tar.gz https://github.com/apache/thrift/archive/refs/tags/v0.14.1.tar.gz && \
    tar -xf thrift-0.14.1.tar.gz &&                                                                      \
    yum install -y flex bison &&                                                                         \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                       \
        /usr/local/cmake-3.20.3/bin/cmake                                                                \
        -Wno-dev                                                                                         \
        -G Ninja                                                                                         \
        -DCMAKE_BUILD_TYPE=Release                                                                       \
        -DCMAKE_INSTALL_PREFIX=/usr/local/thrift-0.14.1                                                  \
        -DBUILD_SHARED_LIBS=OFF                                                                          \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                             \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                  \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                                \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                           \
        -DBOOST_ROOT=/usr/local/boost-1.76.0                                                             \
        -DOPENSSL_ROOT_DIR=/usr/local/openssl-1.1.1                                                      \
        -DLIBEVENT_ROOT=/usr/local/libevent-2.1.12                                                       \
        -DWITH_BOOST_SMART_PTR=OFF                                                                       \
        -DWITH_BOOST_FUNCTIONAL=OFF                                                                      \
        -DWITH_BOOST_STATIC=ON                                                                           \
        -DWITH_JAVA=OFF                                                                                  \
        -DWITH_JAVASCRIPT=OFF                                                                            \
        -DWITH_NODEJS=OFF                                                                                \
        -DWITH_PYTHON=OFF                                                                                \
        -DWITH_SHARED_LIB=OFF                                                                            \
        -DBUILD_TESTING=OFF                                                                              \
        -DBUILD_EXAMPLES=OFF                                                                             \
        -DBUILD_TUTORIALS=OFF                                                                            \
        -Hthrift-0.14.1                                                                                  \
        -Bthrift-0.14.1-build &&                                                                         \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                       \
        /usr/local/cmake-3.20.3/bin/cmake                                                                \
        --build thrift-0.14.1-build                                                                      \
        --config Release --target install &&                                                             \
    rm -rf thrift-0.14.1-build &&                                                                        \
    rm -rf thrift-0.14.1 &&                                                                              \
    rm -f thrift-0.14.1.tar.gz &&                                                                        \
    popd &&                                                                                              \
    ln -svf thrift-0.14.1 /usr/local/thrift &&                                                           \
    echo OK: thrift

RUN pushd /tmp &&                                             \
    curl -L -O https://curl.se/download/curl-7.77.0.tar.gz && \
    tar -xf curl-7.77.0.tar.gz       &&                       \
    cd curl-7.77.0 &&                                         \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64            \
        CC=/usr/local/gcc-7.3.0/bin/gcc                       \
        CXX=/usr/local/gcc-7.3.0/bin/g++                      \
        CFLAGS=-fPIC                                          \
        ./configure                                           \
        --prefix=/usr/local/curl-7.77.0                       \
        --with-ssl=/usr/local/openssl-1.1.1                   \
        --disable-shared                                      \
        --disable-ldap                                        \
        --without-brotli                                      \
        --without-libidn2  &&                                 \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64            \
        CC=/usr/local/gcc-7.3.0/bin/gcc                       \
        CXX=/usr/local/gcc-7.3.0/bin/g++                      \
        CFLAGS=-fPIC                                          \
        make                                                  \
        -j$(nproc) &&                                         \
    env LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64            \
        CC=/usr/local/gcc-7.3.0/bin/gcc                       \
        CXX=/usr/local/gcc-7.3.0/bin/g++                      \
        CFLAGS=-fPIC                                          \
        make                                                  \
        install &&                                            \
    cd .. &&                                                  \
    rm -rf curl-7.77.0 &&                                     \
    rm -f curl-7.77.0.tar.gz &&                               \
    popd &&                                                   \
    ln -svf curl-7.77.0 /usr/local/curl &&                    \
    echo OK: curl

RUN pushd /tmp &&                                                                                                     \
    curl -L -o aws-sdk-cpp-1.7.108.tar.gz https://github.com/aws/aws-sdk-cpp/archive/refs/tags/1.7.108.tar.gz &&      \
    tar -xf aws-sdk-cpp-1.7.108.tar.gz &&                                                                             \
    sed -i -e 's@ "-Werror"@@' aws-sdk-cpp-1.7.108/cmake/compiler_settings.cmake &&                                   \
    sed -i -e 's@UPDATE_COMMAND ""@UPDATE_COMMAND "" PATCH_COMMAND sed -i -e "s% -Werror%%g" cmake/AwsCFlags.cmake@g' \
        aws-sdk-cpp-1.7.108/third-party/cmake/BuildAwsCCommon.cmake &&                                                \
    yum install -y git zlib-devel &&                                                                                  \
    env PATH=/usr/local/ninja-1.10.2/bin:/usr/local/cmake-3.20.3/bin:/usr/local/gcc-7.3.0/bin:${PATH}                 \
        PKG_CONFIG_PATH=/usr/local/curl-7.77.0/lib/pkgconfig                                                          \
        LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                    \
        /usr/local/cmake-3.20.3/bin/cmake                                                                             \
        -Wno-dev                                                                                                      \
        -G Ninja                                                                                                      \
        -DCMAKE_BUILD_TYPE=Release                                                                                    \
        -DCMAKE_INSTALL_PREFIX=/usr/local/aws-sdk-cpp-1.7.108                                                         \
        -DBUILD_SHARED_LIBS=OFF                                                                                       \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                                          \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                               \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                                             \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                                        \
        -DOPENSSL_ROOT_DIR=/usr/local/openssl-1.1.1                                                                   \
        -DBUILD_ONLY="s3;meteringmarketplace"                                                                         \
        -DENABLE_TESTING=OFF                                                                                          \
        -Haws-sdk-cpp-1.7.108                                                                                         \
        -Baws-sdk-cpp-1.7.108-build &&                                                                                \
    env PATH=/usr/local/ninja-1.10.2/bin:/usr/local/cmake-3.20.3/bin:/usr/local/gcc-7.3.0/bin:${PATH}                 \
        PKG_CONFIG_PATH=/usr/local/curl-7.77.0/lib/pkgconfig                                                          \
        LD_LIBRARY_PATH=/usr/local/gcc-7.3.0/lib64                                                                    \
        /usr/local/cmake-3.20.3/bin/cmake                                                                             \
        --build aws-sdk-cpp-1.7.108-build                                                                             \
        --config Release --target install &&                                                                          \
    rm -rf aws-sdk-cpp-1.7.108-build &&                                                                               \
    rm -rf aws-sdk-cpp-1.7.108 &&                                                                                     \
    rm -f aws-sdk-cpp-1.7.108.tar.gz &&                                                                               \
    popd &&                                                                                                           \
    ln -svf aws-sdk-cpp-1.7.108 /usr/local/aws-sdk-cpp &&                                                             \
    echo OK: aws-sdk-cpp

RUN pushd /tmp &&                                                                                                                       \
    mkdir -p python-3.7.7-build &&                                                                                                      \
    cd python-3.7.7-build &&                                                                                                            \
    curl -L -o pyenv.run https://pyenv.run &&                                                                                           \
    env PYENV_ROOT=/usr/local/python-3.7.7/.pyenv bash pyenv.run &&                                                                     \
    yum install -y which gcc gcc-c++ make zlib-devel bzip2 bzip2-devel readline-devel                                                   \
        sqlite sqlite-devel openssl-devel tk-devel libffi-devel xz-devel &&                                                             \
    env PYENV_ROOT=/usr/local/python-3.7.7/.pyenv /usr/local/python-3.7.7/.pyenv/bin/pyenv install 3.7.7 &&                             \
    env PYENV_ROOT=/usr/local/python-3.7.7/.pyenv /usr/local/python-3.7.7/.pyenv/bin/pyenv global 3.7.7 &&                              \
    mv /usr/local/python-3.7.7/.pyenv/versions/3.7.7/{bin,lib,include,share} /usr/local/python-3.7.7/ &&                                \
    rm -rf /usr/local/python-3.7.7/.pyenv &&                                                                                            \
    /usr/local/python-3.7.7/bin/python3.7 -m pip install --upgrade pip setuptools wheel &&                                              \
    /usr/local/python-3.7.7/bin/python3.7 -m pip install --upgrade numpy==1.20.3 &&                                                     \
    /usr/local/python-3.7.7/bin/python3.7 -m pip install --upgrade torch==1.7.1+cpu torchvision==0.8.2+cpu                              \
        -f https://download.pytorch.org/whl/torch_stable.html &&                                                                        \
    /usr/local/python-3.7.7/bin/python3.7 -m pip install --upgrade faiss-cpu==1.7.1.post2 &&                                            \
    /usr/local/python-3.7.7/bin/python3.7 -m pip install --upgrade awscli awscli-plugin-endpoint &&                                     \
    find /usr/local/python-3.7.7/bin -type f -exec grep '^#!.\+/bin/python\(3\(\.7\)\?\)\?$' {} -Hn --color \;  &&                      \
    find /usr/local/python-3.7.7/bin -type f -exec sed -i -e 's@^#!.\+/bin/python\(3\(\.7\)\?\)\?$@#!/usr/bin/env python3.7@' {} \;  && \
    cp /usr/local/gcc-7.3.0/lib64/libgcc_s.so.1 /usr/local/python-3.7.7/lib &&                                                          \
    cp /usr/local/gcc-7.3.0/lib64/libstdc++.so.6 /usr/local/python-3.7.7/lib &&                                                         \
    chmod a+x /usr/local/python-3.7.7/lib/libgcc_s.so.1 &&                                                                              \
    chmod a+x /usr/local/python-3.7.7/lib/libstdc++.so.6 &&                                                                             \
    tar -czf /usr/local/python-env-3.7.7.tgz -C /usr/local/python-3.7.7 $(ls /usr/local/python-3.7.7) &&                                \
    cd .. &&                                                                                                                            \
    rm -rf python-3.7.7-build &&                                                                                                        \
    popd &&                                                                                                                             \
    ln -svf python-3.7.7 /usr/local/python &&                                                                                           \
    ln -svf python-env-3.7.7.tgz /usr/local/python-env.tgz &&                                                                           \
    echo OK: python

RUN pushd /tmp &&                                                                                             \
    curl -L -o dbg-macro-0.4.0.tar.gz https://github.com/sharkdp/dbg-macro/archive/refs/tags/v0.4.0.tar.gz && \
    tar -xf dbg-macro-0.4.0.tar.gz &&                                                                         \
    /usr/local/cmake-3.20.3/bin/cmake                                                                         \
        -Wno-dev                                                                                              \
        -G Ninja                                                                                              \
        -DCMAKE_BUILD_TYPE=Release                                                                            \
        -DCMAKE_INSTALL_PREFIX=/usr/local/dbg-macro-0.4.0                                                     \
        -DBUILD_SHARED_LIBS=OFF                                                                               \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON                                                                  \
        -DCMAKE_C_COMPILER=/usr/local/gcc-7.3.0/bin/gcc                                                       \
        -DCMAKE_CXX_COMPILER=/usr/local/gcc-7.3.0/bin/g++                                                     \
        -DCMAKE_MAKE_PROGRAM=/usr/local/ninja-1.10.2/bin/ninja                                                \
        -DDBG_MACRO_ENABLE_TESTS=OFF                                                                          \
        -Hdbg-macro-0.4.0                                                                                     \
        -Bdbg-macro-0.4.0-build &&                                                                            \
    /usr/local/cmake-3.20.3/bin/cmake                                                                         \
        --build dbg-macro-0.4.0-build                                                                         \
        --config Release --target install &&                                                                  \
    rm -rf dbg-macro-0.4.0-build &&                                                                           \
    rm -rf dbg-macro-0.4.0 &&                                                                                 \
    rm -f dbg-macro-0.4.0.tar.gz &&                                                                           \
    popd &&                                                                                                   \
    ln -svf dbg-macro-0.4.0 /usr/local/dbg-macro &&                                                           \
    echo OK: dbg-macro
