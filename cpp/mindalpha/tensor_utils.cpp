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

#include <sstream>
#include <mindalpha/tensor_utils.h>

namespace mindalpha
{

size_t SliceElements(const std::vector<size_t>& shape)
{
    if (shape.empty())
        return 0;
    size_t n = 1;
    for (size_t i = 1; i < shape.size(); i++)
        n *= shape[i];
    return n;
}

size_t TotalElements(const std::vector<size_t>& shape)
{
    if (shape.empty())
        return 0;
    size_t n = 1;
    for (size_t i = 0; i < shape.size(); i++)
        n *= shape[i];
    return n;
}

std::string ShapeToString(const std::vector<size_t>& shape)
{
    std::ostringstream sout;
    for (size_t i = 0; i < shape.size(); i++)
        sout << (i ? " " : "") << shape.at(i);
    return sout.str();
}

std::vector<size_t> ShapeFromString(const std::string& str)
{
    std::vector<size_t> shape;
    std::istringstream sin(str);
    size_t dim;
    while (sin >> dim)
        shape.push_back(dim);
    return shape;
}

}
