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

#include <memory>
#include <stdexcept>
#include <mindalpha/pybind_utils.h>
#include <mindalpha/combine_schema.h>
#include <mindalpha/index_batch.h>
#include <mindalpha/hash_uniquifier.h>
#include <mindalpha/feature_extraction_python_bindings.h>

namespace py = pybind11;

namespace mindalpha
{

void DefineFeatureExtractionBindings(pybind11::module& m)
{

py::class_<mindalpha::CombineSchema, std::shared_ptr<mindalpha::CombineSchema>>(m, "CombineSchema")
    .def_property_readonly("feature_count", &mindalpha::CombineSchema::GetFeatureCount)
    .def_property_readonly("column_name_source", &mindalpha::CombineSchema::GetColumnNameSource)
    .def_property_readonly("combine_schema_source", &mindalpha::CombineSchema::GetCombineSchemaSource)
    .def(py::init<>())
    .def("clear", &mindalpha::CombineSchema::Clear)
    .def("load_column_name_from_source", &mindalpha::CombineSchema::LoadColumnNameFromSource)
    .def("load_column_name_from_file", &mindalpha::CombineSchema::LoadColumnNameFromFile)
    .def("load_combine_schema_from_source", &mindalpha::CombineSchema::LoadCombineSchemaFromSource)
    .def("load_combine_schema_from_file", &mindalpha::CombineSchema::LoadCombineSchemaFromFile)
    .def("get_column_name_map", [](const mindalpha::CombineSchema& schema)
         {
             py::dict map;
             for (auto&& [key, value] : schema.GetColumnNameMap())
                 map[py::str(key)] = value;
             return map;
         })
    .def("combine_to_indices_and_offsets",
         [](const mindalpha::CombineSchema& schema, const mindalpha::IndexBatch& batch, bool feature_offset)
         {
             auto [indices, offsets] = schema.CombineToIndicesAndOffsets(batch, feature_offset);
             py::array indices_arr = mindalpha::to_numpy_array(std::move(indices));
             py::array offsets_arr = mindalpha::to_numpy_array(std::move(offsets));
             return py::make_tuple(indices_arr, offsets_arr);
         })
    .def_static("compute_feature_hash", [](py::object feature)
        {
            std::vector<std::pair<std::string, std::string>> vec;
            for (const auto& item : feature)
            {
                const py::tuple t = item.cast<py::tuple>();
                std::string name = t[0].cast<std::string>();
                std::string value = t[1].cast<std::string>();
                if (value == "none")
                    throw std::runtime_error("none as value is invalid, because it should have been filtered");
                vec.emplace_back(std::move(name), std::move(value));
            }
            return mindalpha::CombineSchema::ComputeFeatureHash(vec);
        })
    .def(py::pickle(
        [](const mindalpha::CombineSchema& schema)
        {
            auto& str1 = schema.GetColumnNameSource();
            auto& str2 = schema.GetCombineSchemaSource();
            return py::make_tuple(str1, str2);
        },
        [](py::tuple t)
        {
            if (t.size() != 2)
                throw std::runtime_error("invalid pickle state");
            std::string str1 = t[0].cast<std::string>();
            std::string str2 = t[1].cast<std::string>();
            auto schema = std::make_shared<mindalpha::CombineSchema>();
            schema->LoadColumnNameFromSource(str1);
            schema->LoadCombineSchemaFromSource(str2);
            return schema;
        }))
    ;

py::class_<mindalpha::IndexBatch, std::shared_ptr<mindalpha::IndexBatch>>(m, "IndexBatch")
    .def_property_readonly("rows", &mindalpha::IndexBatch::GetRows)
    .def_property_readonly("columns", &mindalpha::IndexBatch::GetColumns)
    .def(py::init<py::list, const std::string&>())
    .def("to_list", &mindalpha::IndexBatch::ToList)
    .def("__str__", &mindalpha::IndexBatch::ToString)
    ;

py::class_<mindalpha::HashUniquifier>(m, "HashUniquifier")
    .def_static("uniquify", [](py::array_t<uint64_t> items)
        {
            std::vector<uint64_t> entries = HashUniquifier::Uniquify(items.mutable_data(), items.size());
            return mindalpha::to_numpy_array(std::move(entries));
        })
    ;

}

}
