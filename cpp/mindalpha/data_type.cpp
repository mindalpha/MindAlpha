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

#include <stdexcept>
#include <spdlog/spdlog.h>
#include <mindalpha/data_type.h>
#include <mindalpha/stack_trace_utils.h>

namespace mindalpha
{

std::string DataTypeToString(DataType type)
{
    switch (type)
    {
#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u) case DataType::u: return #l;
    MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)
    default:
        std::string serr;
        serr.append("Invalid DataType enum value: ");
        serr.append(std::to_string(static_cast<int>(type)));
        serr.append(".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
}

DataType DataTypeFromString(const std::string& str)
{
#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u) if (str == #l) return DataType::u;
    MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)
    std::string serr;
    serr.append("Invalid DataType enum value: ");
    serr.append(str);
    serr.append(".\n\n");
    serr.append(GetStackTrace());
    spdlog::error(serr);
    throw std::runtime_error(serr);
}

std::string NullableDataTypeToString(DataType type)
{
    if (type == NullDataType)
        return NullDataTypeString;
    return DataTypeToString(type);
}

DataType NullableDataTypeFromString(const std::string& str)
{
    if (str == NullDataTypeString)
        return NullDataType;
    return DataTypeFromString(str);
}

size_t DataTypeToSize(DataType type)
{
    switch (type)
    {
#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u) case DataType::u: return sizeof(t);
    MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)
    default:
        std::string serr;
        serr.append("Invalid DataType enum value: ");
        serr.append(std::to_string(static_cast<int>(type)));
        serr.append(".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
}

UserOption::UserOption(const std::string& str) {
    std::string err;
    if (str.empty()) {
        return;
    }
    json = json11::Json::parse(str, err);
    if (!err.empty()) {
        std::string serr("__UserOption__ ");
        serr.append(str);
        serr.append("Parse Fail");
        throw std::runtime_error(serr);
    }
    enable_half_float = json["enable_half_float"].bool_value();
    half_float_type = json["half_float_type"].string_value();
    scalar = json["scalar"].int_value();
}
std::string UserOption::ToString() const {
    return json.dump();
}
}
