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

#include <mindalpha/string_utils.h>
#include <mindalpha/pybind_utils.h>
#include <pybind11/numpy.h>

namespace mindalpha
{

class __attribute__((visibility("hidden"))) IndexBatch
{
public:
    IndexBatch(const std::string& schema_file);
    IndexBatch(pybind11::list columns, const std::string& delimiters);
    IndexBatch(pybind11::list column_names, pybind11::list columns, const std::string& delimiters);

    void ConvertColumn(pybind11::list columns, const std::string& delimiters);
    const StringViewHashVector& GetCell(size_t i, size_t j, const std::string& column_name) const;

    pybind11::list ToList() const;

    size_t GetRows() const { return rows_; }
    size_t GetColumns() const { return split_columns_.size(); }

    std::string ToString() const;
    size_t GetColumnNameSize() {
        return column_names_.size();
    }
private:
    struct __attribute__((visibility("hidden"))) string_view_cell
    {
        StringViewHashVector items_;
        pybind11::object obj_;
    };

    using StringViewColumn = std::vector<string_view_cell>;

    static StringViewColumn SplitColumn(const pybind11::array& column, std::string_view delims);

    std::vector<StringViewColumn> split_columns_;
    size_t rows_;

    std::unordered_map<std::string, int> column_name_map_;
    std::vector<std::string> column_names_;
};

}
