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

#include <string.h>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <mindalpha/stack_trace_utils.h>
#include <mindalpha/hashtable_helpers.h>
#include <mindalpha/map_file_header.h>

namespace mindalpha
{

const char map_file_signature[map_file_signature_size] = "\x89MemoryMappedArrayHashMap\0\0\0\0\0\0";
const uint64_t map_file_version = 0x0000000000000004;

void MapFileHeader::FillBasicFields()
{
    memcpy(signature, map_file_signature, map_file_signature_size);
    version = map_file_version;
    reserved_ = 0;
}

bool MapFileHeader::IsSignatureValid() const
{
    return memcmp(signature, map_file_signature, map_file_signature_size) == 0;
}

void MapFileHeader::Validate(const std::string& hint) const
{
    if (!IsSignatureValid())
    {
        std::string serr;
        serr.append(hint);
        serr.append("file signature not match.\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
    if (version != map_file_version)
    {
        std::string serr;
        serr.append(hint);
        serr.append("file version not match, expect " + std::to_string(map_file_version) + ", ");
        serr.append("found " + std::to_string(version) + ".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
    if (static_cast<int64_t>(value_count_per_key) < 0)
    {
        std::string serr;
        serr.append(hint);
        serr.append("value_count_per_key must be non-negative ");
        serr.append(std::to_string(static_cast<int64_t>(value_count_per_key)) + ".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
    if (key_count * value_count_per_key != value_count)
    {
        std::string serr;
        serr.append(hint);
        serr.append("value_count is incorrect. ");
        serr.append("key_count = " + std::to_string(key_count) + ", ");
        serr.append("value_count = " + std::to_string(value_count) + ", ");
        serr.append("value_count_per_key = " + std::to_string(value_count_per_key) + ".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
    if (reserved_ != 0)
    {
        std::string serr;
        serr.append(hint);
        serr.append("reserved_ field not zero. ");
        serr.append("reserved_ = " + std::to_string(reserved_) + ".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
    if (key_count > bucket_count)
    {
        std::string serr;
        serr.append(hint);
        serr.append("key_count exceeds bucket_count. ");
        serr.append("key_count = " + std::to_string(key_count) + ", ");
        serr.append("bucket_count = " + std::to_string(bucket_count) + ".\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
    if (bucket_count > 0)
    {
        if (HashtableHelpers::GetPowerBucketCount(bucket_count) != bucket_count)
        {
            std::string serr;
            serr.append(hint);
            serr.append("bucket_count " + std::to_string(bucket_count) + " is invalid; ");
            serr.append("it must be a power of 2.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
    }
}

}
