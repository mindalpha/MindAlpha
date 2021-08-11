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
#include <mindalpha/index_batch.h>
#include <mindalpha/debug.h>

namespace mindalpha
{
IndexBatch::IndexBatch(const std::string& schema_file) {

}
IndexBatch::IndexBatch(pybind11::list column_names, pybind11::list columns, const std::string& delimiters) {
    if (columns.size() <= 0) {
        throw std::runtime_error("empty columns list");
    }
    column_names_.reserve(column_names.size());
    for (auto & col : column_names) {
        //https://github.com/pybind/pybind11/issues/1201
        auto o = pybind11::cast<pybind11::object>(col);
        auto [str, obj] = get_string_object_tuple(o);
        // todo compare with string_view
        std::string col_name(str.data(), str.size());
        column_name_map_.emplace(col_name, column_names_.size());
        column_names_.emplace_back(col_name);
    }
    ConvertColumn(columns, delimiters);
}
IndexBatch::IndexBatch(pybind11::list columns, const std::string& delimiters) {
   ConvertColumn(std::move(columns), delimiters);
}
void IndexBatch::ConvertColumn(pybind11::list columns, const std::string& delimiters)
{
    if (columns.empty()) {
        throw std::runtime_error("empty columns list");
    }
    split_columns_.reserve(columns.size());
    size_t rows = 0;
    for (size_t j = 0; j < columns.size(); j++)
    {
        pybind11::object item = columns[j];
        if (!pybind11::isinstance<pybind11::array>(item)) {
            throw std::runtime_error(fmt::format("column {} is not numpy ndarray, but {}", j));
        }
        pybind11::array arr = item.cast<pybind11::array>();
        if (arr.dtype().kind() != 'O') {
            throw std::runtime_error(fmt::format("column {} is not numpy ndarray of object", j));
        }
        StringViewColumn column = SplitColumn(arr, delimiters);
        if (j == 0) {
            rows = column.size();
        }
        if (column.size() != rows) {
            throw std::runtime_error(fmt::format("column {} and column 0 are not of the same length; {} != {}",
                                     j, column.size(), rows));
        }
        split_columns_.push_back(std::move(column));
    }
    if (rows == 0) {
        throw std::runtime_error("number of rows is zero");
    }
    rows_ = rows;
}

IndexBatch::StringViewColumn
IndexBatch::SplitColumn(const pybind11::array& column, std::string_view delims)
{
    const size_t rows = column.size();
    StringViewColumn output;
    output.reserve(rows);
#if 1
    for (auto& item: column) {
#else
    for (size_t i = 0; i < rows; i++)
    {
        const void* item_ptr = column.data(i);
        // Consider avoiding complex casting here.
        PyObject* item = (PyObject*)(*(void**)item_ptr);
#endif
        pybind11::object cell = pybind11::reinterpret_borrow<pybind11::object>(item);
        auto [str, obj] = get_string_object_tuple(cell);
        auto items = SplitFilterStringViewHash(str, delims);
        output.push_back(string_view_cell{std::move(items), std::move(obj)});
    }
    return output;
}

const StringViewHashVector& IndexBatch::GetCell(size_t i, size_t j, const std::string& column_name) const
{
    if (i >= rows_) {
        throw std::runtime_error(fmt::format("row index i is out of range; {}>={}", i, rows_));
    }
    if (column_names_.size()) {
        auto iter = column_name_map_.find(column_name);
        if (iter == column_name_map_.end()) {
            throw std::runtime_error(fmt::format("can't find {} in column_name_map element_size {}",
                                    column_name, column_name_map_.size()));
        }
        j = iter->second;
    }
    if (j >= split_columns_.size()) {
        throw std::runtime_error(fmt::format("column index j ({}) is out of range; {} >={}",
                                  column_name, j, split_columns_.size()));
    }
    auto& column = split_columns_.at(j);
    return column.at(i).items_;
}

pybind11::list IndexBatch::ToList() const
{
    pybind11::list rows;
    for (size_t i = 0; i < rows_; i++)
    {
        pybind11::list row;
        for (size_t j = 0; j < split_columns_.size(); j++)
        {
            pybind11::list items;
            auto& cell = split_columns_.at(j).at(i).items_;
            for (size_t k = 0; k < cell.size(); k++)
            {
                auto item = pybind11::make_tuple(cell.at(k).view_, cell.at(k).hash_);
                items.append(item);
            }
            row.append(items);
        }
        rows.append(row);
    }
    return rows;
}

std::string IndexBatch::ToString() const
{
    return fmt::format("[IndexBatch: {} x {}]", GetRows(), GetColumns());
}

}
