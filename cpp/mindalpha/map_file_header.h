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
#include <mindalpha/data_type.h>

//
// ``map_file_header.h`` defines the binary structure of ArrayHashMap
// file header. For simplicity and efficiency, we assume little endian
// and do not consider portability.
//

namespace mindalpha
{

const uint64_t map_file_signature_size = 32;

struct MapFileHeader
{
    char signature[map_file_signature_size];
    uint64_t version;
    uint64_t reserved_; // for backward compatibility
    uint64_t key_type;
    uint64_t value_type;
    uint64_t key_count;
    uint64_t bucket_count;
    uint64_t value_count;
    uint64_t value_count_per_key;

    void FillBasicFields();
    bool IsSignatureValid() const;
    void Validate(const std::string& hint) const;
};

const uint64_t map_file_header_size = sizeof(MapFileHeader);

extern const char map_file_signature[map_file_signature_size];
extern const uint64_t map_file_version;

}
