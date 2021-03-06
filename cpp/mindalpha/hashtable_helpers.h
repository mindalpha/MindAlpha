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

//
// ``hashtable_helpers.h`` contains helper functions useful for
// implementing hashtables.
//

namespace mindalpha
{

class HashtableHelpers
{
public:
    static constexpr uint64_t GetPowerBucketCount(uint64_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;
        return v;
    }

    static constexpr uint64_t FastModulo(uint64_t a)
    {
        constexpr int q = 31;
        constexpr uint64_t prime = (UINT64_C(1) << q) - 1;
        uint64_t r = (a & prime) + (a >> q);
        if (r >= prime)
            r -= prime;
        return r;
    }
};

}
