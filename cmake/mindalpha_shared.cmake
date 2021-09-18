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

get_project_version(project_version)
message(STATUS "project_version: ${project_version}")

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/gen/thrift/cpp/mindalpha)

add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/gen/thrift/cpp/mindalpha/message_meta_types.h
           ${PROJECT_BINARY_DIR}/gen/thrift/cpp/mindalpha/message_meta_types.cpp
    COMMAND thrift -gen cpp:cob_style,moveable_types
            -out ${PROJECT_BINARY_DIR}/gen/thrift/cpp/mindalpha
            ${PROJECT_SOURCE_DIR}/thrift/mindalpha/message_meta.thrift
    DEPENDS ${PROJECT_SOURCE_DIR}/thrift/mindalpha/message_meta.thrift)

add_library(mindalpha_shared SHARED
    cpp/mindalpha/stack_trace_utils.h
    cpp/mindalpha/stack_trace_utils.cpp
    cpp/mindalpha/thread_utils.h
    cpp/mindalpha/thread_utils.cpp
    cpp/mindalpha/string_utils.h
    cpp/mindalpha/vector_utils.h
    cpp/mindalpha/hashtable_helpers.h
    cpp/mindalpha/hash_uniquifier.h
    cpp/mindalpha/hash_uniquifier.cpp
    cpp/mindalpha/data_type.h
    cpp/mindalpha/data_type.cpp
    cpp/mindalpha/smart_array.h
    cpp/mindalpha/memory_buffer.h
    cpp/mindalpha/map_file_header.h
    cpp/mindalpha/map_file_header.cpp
    cpp/mindalpha/array_hash_map.h
    cpp/mindalpha/node_role.h
    cpp/mindalpha/node_role.cpp
    cpp/mindalpha/node_encoding.h
    cpp/mindalpha/node_encoding.cpp
    cpp/mindalpha/node_info.h
    cpp/mindalpha/node_info.cpp
    cpp/mindalpha/node_control_command.h
    cpp/mindalpha/node_control_command.cpp
    cpp/mindalpha/node_control.h
    cpp/mindalpha/node_control.cpp
    cpp/mindalpha/message_meta.h
    cpp/mindalpha/message_meta.cpp
    cpp/mindalpha/message.h
    cpp/mindalpha/message.cpp

    cpp/mindalpha/actor_config.cpp
    cpp/mindalpha/message_transport.cpp
    cpp/mindalpha/zeromq_transport.cpp
    cpp/mindalpha/actor_process.cpp
    cpp/mindalpha/node_manager.cpp
    cpp/mindalpha/network_utils.cpp
    cpp/mindalpha/ps_agent.cpp
    cpp/mindalpha/ps_runner.cpp
    cpp/mindalpha/io.cpp
    cpp/mindalpha/filesys.cpp
    cpp/mindalpha/local_filesys.cpp
    cpp/mindalpha/s3_sdk_filesys.cpp
    ${PROJECT_BINARY_DIR}/gen/thrift/cpp/mindalpha/message_meta_types.h
    ${PROJECT_BINARY_DIR}/gen/thrift/cpp/mindalpha/message_meta_types.cpp
    cpp/mindalpha/dense_tensor_meta.cpp
    cpp/mindalpha/dense_tensor_partition.cpp
    cpp/mindalpha/sparse_tensor_meta.cpp
    cpp/mindalpha/sparse_tensor_partition.cpp
    cpp/mindalpha/array_hash_map_reader.h
    cpp/mindalpha/array_hash_map_writer.h
    cpp/mindalpha/tensor_partition_store.cpp
    cpp/mindalpha/dense_tensor.cpp
    cpp/mindalpha/sparse_tensor.cpp
    cpp/mindalpha/ps_default_agent.cpp
    cpp/mindalpha/ps_helper.cpp
    cpp/mindalpha/combine_schema.cpp
    cpp/mindalpha/index_batch.cpp
    cpp/mindalpha/hash_uniquifier.cpp
    cpp/mindalpha/model_metric_buffer.cpp
    cpp/mindalpha/tensor_utils.cpp
    cpp/mindalpha/pybind_utils.cpp
    cpp/mindalpha/ml_ps_python_bindings.cpp
    cpp/mindalpha/tensor_store_python_bindings.cpp
    cpp/mindalpha/feature_extraction_python_bindings.cpp
)
set_target_properties(mindalpha_shared PROPERTIES PREFIX "")
set_target_properties(mindalpha_shared PROPERTIES OUTPUT_NAME _mindalpha)
target_compile_features(mindalpha_shared PRIVATE cxx_std_17)
target_compile_definitions(mindalpha_shared PRIVATE DMLC_USE_S3=1)
target_compile_definitions(mindalpha_shared PRIVATE _MINDALPHA_VERSION="${project_version}")
target_compile_definitions(mindalpha_shared PRIVATE DBG_MACRO_NO_WARNING)
target_include_directories(mindalpha_shared PRIVATE ${PROJECT_SOURCE_DIR}/cpp)
target_include_directories(mindalpha_shared PRIVATE ${PROJECT_BINARY_DIR}/gen/thrift/cpp)
target_link_libraries(mindalpha_shared PRIVATE
    json11_static
    pybind11::pybind11
    Python::Module
    aws-cpp-sdk-s3
    aws-cpp-sdk-core
    spdlog::spdlog
    Boost::headers
    thrift::thrift
    zmq::libzmq
)
