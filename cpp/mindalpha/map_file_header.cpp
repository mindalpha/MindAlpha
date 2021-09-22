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
#include <sstream>
#include <stdexcept>
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
        std::ostringstream serr;
        serr << hint;
        serr << "file signature not match.";
        throw std::runtime_error(serr.str());
    }
    if (version != map_file_version)
    {
        std::ostringstream serr;
        serr << hint;
        serr << "file version not match, expect " << map_file_version << ", ";
        serr << "found " << version << ".";
        throw std::runtime_error(serr.str());
    }
    if (static_cast<int64_t>(value_count_per_key) < 0)
    {
        std::ostringstream serr;
        serr << hint;
        serr << "value_count_per_key must be non-negative ";
        serr << static_cast<int64_t>(value_count_per_key) << ".";
        throw std::runtime_error(serr.str());
    }
    if (key_count * value_count_per_key != value_count)
    {
        std::ostringstream serr;
        serr << hint;
        serr << "value_count is incorrect. ";
        serr << "key_count = " << key_count << ", ";
        serr << "value_count = " << value_count << ", ";
        serr << "value_count_per_key = " << value_count_per_key << ".";
        throw std::runtime_error(serr.str());
    }
    if (reserved_ != 0)
    {
        std::ostringstream serr;
        serr << hint;
        serr << "reserved_ field not zero. ";
        serr << "reserved_ = " << reserved_ << ".";
        throw std::runtime_error(serr.str());
    }
    if (key_count > bucket_count)
    {
        std::ostringstream serr;
        serr << hint;
        serr << "key_count exceeds bucket_count. ";
        serr << "key_count = " << key_count << ", ";
        serr << "bucket_count = " << bucket_count << ".";
        throw std::runtime_error(serr.str());
    }
    if (bucket_count > 0)
    {
        if (HashtableHelpers::GetPowerBucketCount(bucket_count) != bucket_count)
        {
            std::ostringstream serr;
            serr << hint;
            serr << "bucket_count " << bucket_count << " is invalid; ";
            serr << "it must be a power of 2.";
            throw std::runtime_error(serr.str());
        }
    }
}

}
