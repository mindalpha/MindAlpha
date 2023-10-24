//
// Copyright 2021 Mobvista
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <stdint.h>
#include <string>
#include <json11.hpp>
#include <iostream>

//
// ``data_type.h`` defines enum ``DataType`` to represent numeric data types
// and some helper functions to convert ``DataType`` values.
//
namespace mindalpha
{

//
// Use the X Macro technique to simplify code. See the following page
// for more information about X Macros:
//
//   https://en.wikipedia.org/wiki/X_Macro
//

#define MINDALPHA_INTEGRAL_DATA_TYPES(X)  \
    X(int8_t,   int8,    Int8)            \
    X(int16_t,  int16,   Int16)           \
    X(int32_t,  int32,   Int32)           \
    X(int64_t,  int64,   Int64)           \
    X(uint8_t,  uint8,   UInt8)           \
    X(uint16_t, uint16,  UInt16)          \
    X(uint32_t, uint32,  UInt32)          \
    X(uint64_t, uint64,  UInt64)          \
    /**/

#define MINDALPHA_FLOATING_DATA_TYPES(X)             \
    X(float,               float32, Float32)         \
    X(double,              float64, Float64)         \
    /**/

#define MINDALPHA_DATA_TYPES(X)           \
    MINDALPHA_INTEGRAL_DATA_TYPES(X)      \
    MINDALPHA_FLOATING_DATA_TYPES(X)      \
    /**/

enum class DataType
{
#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u) u,
    MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)
};

// A missing ``DataType`` is represented by ``DataType(-1)``.
constexpr DataType NullDataType = static_cast<DataType>(-1);
constexpr const char* NullDataTypeString = "null";

// Functions to convert ``DataType`` to and from strings.
std::string DataTypeToString(DataType type);
DataType DataTypeFromString(const std::string& str);

std::string NullableDataTypeToString(DataType type);
DataType NullableDataTypeFromString(const std::string& str);

// This class template computes the ``DataType`` code of a numeric type.
template<typename T>
struct DataTypeToCode;

#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u)                \
    template<>                                          \
    struct DataTypeToCode<t>                            \
    {                                                   \
        static constexpr DataType value = DataType::u;  \
    };                                                  \
    /**/
    MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)

// Compute the size in bytes of a value of ``type``.
size_t DataTypeToSize(DataType type);

// This function template and two function overloads ensure ``value``
// can be output as numbers. Output ``int8_t``/``uint8_t`` directly to
// ``std::ostream`` will cause problems as they are character types
// actually.
template<typename T>
inline T AsNumber(T value) { return value; }

inline int32_t AsNumber(int8_t value) { return static_cast<int32_t>(value); }
inline uint32_t AsNumber(uint8_t value) { return static_cast<uint32_t>(value); }

class UserOption {
  public:
    UserOption(const std::string& str);
    std::string ToString() const;
  public:
    int type;
    bool enable_half_float;
    int scalar;
    std::string half_float_type;
    json11::Json json;
};
}
