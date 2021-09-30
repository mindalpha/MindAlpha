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

FROM ubuntu:20.04

ENV LANG=C
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get upgrade -y
RUN ln -svf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && env DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata
RUN apt-get install --fix-missing -y build-essential curl cmake patchelf python3 python3-pip ninja-build git wget gnupg2
RUN apt-get update

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 30
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 30
RUN python -m pip install --upgrade awscli

RUN apt-get install -y pkg-config                                        \
        libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev \
        libboost-dev pybind11-dev libjson11-1-dev libfmt-dev             \
        libspdlog-dev libthrift-dev thrift-compiler libzmq5-dev

RUN git clone https://github.com/zeromq/cppzmq.git /tmp/cppzmq &&         \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
          -DCPPZMQ_BUILD_TESTS=OFF                                        \
          -H/tmp/cppzmq -B/tmp/cppzmq/build &&                            \
    cmake --build /tmp/cppzmq/build --target install &&                   \
    rm -rf /tmp/cppzmq

RUN git clone https://github.com/sharkdp/dbg-macro.git /tmp/dbg-macro &&  \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
          -DDBG_MACRO_ENABLE_TESTS=OFF                                    \
          -H/tmp/dbg-macro -B/tmp/dbg-macro/build &&                      \
    cmake --build /tmp/dbg-macro/build --target install &&                \
    rm -rf /tmp/dbg-macro

RUN git clone -b 1.7.108 https://github.com/aws/aws-sdk-cpp.git /tmp/aws-sdk-cpp && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr           \
          -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF -DBUILD_ONLY=s3              \
          -H/tmp/aws-sdk-cpp -B/tmp/aws-sdk-cpp/build &&                            \
    cmake --build /tmp/aws-sdk-cpp/build --target install &&                        \
    rm -rf /tmp/aws-sdk-cpp

RUN git clone https://github.com/cameron314/concurrentqueue.git /tmp/concurrentqueue && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr               \
          -H/tmp/concurrentqueue -B/tmp/concurrentqueue/build &&                        \
    cmake --build /tmp/concurrentqueue/build --target install &&                        \
    rm -rf /tmp/concurrentqueue

RUN cd /tmp &&                                                                                                                          \
    mkdir -p python-3.8.5-build &&                                                                                                      \
    cd python-3.8.5-build &&                                                                                                            \
    curl -L -o pyenv.run https://pyenv.run &&                                                                                           \
    env PYENV_ROOT=/usr/local/python-3.8.5/.pyenv bash pyenv.run &&                                                                     \
    apt-get install -y git make build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev                         \
        wget curl llvm libncursesw5-dev xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev &&                            \
    env PYENV_ROOT=/usr/local/python-3.8.5/.pyenv /usr/local/python-3.8.5/.pyenv/bin/pyenv install 3.8.5 &&                             \
    env PYENV_ROOT=/usr/local/python-3.8.5/.pyenv /usr/local/python-3.8.5/.pyenv/bin/pyenv global 3.8.5 &&                              \
    mv /usr/local/python-3.8.5/.pyenv/versions/3.8.5/bin /usr/local/python-3.8.5/ &&                                                    \
    mv /usr/local/python-3.8.5/.pyenv/versions/3.8.5/lib /usr/local/python-3.8.5/ &&                                                    \
    mv /usr/local/python-3.8.5/.pyenv/versions/3.8.5/include /usr/local/python-3.8.5/ &&                                                \
    mv /usr/local/python-3.8.5/.pyenv/versions/3.8.5/share /usr/local/python-3.8.5/ &&                                                  \
    rm -rf /usr/local/python-3.8.5/.pyenv &&                                                                                            \
    /usr/local/python-3.8.5/bin/python3.8 -m pip install --upgrade pip setuptools wheel &&                                              \
    /usr/local/python-3.8.5/bin/python3.8 -m pip install --upgrade numpy==1.20.3 &&                                                     \
    /usr/local/python-3.8.5/bin/python3.8 -m pip install --upgrade torch==1.7.1+cpu torchvision==0.8.2+cpu                              \
        -f https://download.pytorch.org/whl/torch_stable.html &&                                                                        \
    /usr/local/python-3.8.5/bin/python3.8 -m pip install --upgrade faiss-cpu==1.7.1.post2 &&                                            \
    /usr/local/python-3.8.5/bin/python3.8 -m pip install --upgrade awscli awscli-plugin-endpoint &&                                     \
    find /usr/local/python-3.8.5/bin -type f -exec grep '^#!.\+/bin/python\(3\(\.8\)\?\)\?$' {} -Hn --color \;  &&                      \
    find /usr/local/python-3.8.5/bin -type f -exec sed -i -e 's@^#!.\+/bin/python\(3\(\.8\)\?\)\?$@#!/usr/bin/env python3.8@' {} \;  && \
    cp /lib/x86_64-linux-gnu/libzmq.so.5 /usr/local/python-3.8.5/lib &&                                                                 \
    cp /lib/x86_64-linux-gnu/libspdlog.so.1 /usr/local/python-3.8.5/lib &&                                                              \
    cp /lib/x86_64-linux-gnu/libthrift-0.13.0.so /usr/local/python-3.8.5/lib &&                                                         \
    tar -czf /usr/local/python-env-3.8.5.tgz -C /usr/local/python-3.8.5 $(ls /usr/local/python-3.8.5) &&                                \
    cd .. &&                                                                                                                            \
    rm -rf python-3.8.5-build &&                                                                                                        \
    cd &&                                                                                                                               \
    ln -svf python-3.8.5 /usr/local/python &&                                                                                           \
    ln -svf python-env-3.8.5.tgz /usr/local/python-env.tgz &&                                                                           \
    echo OK: python
