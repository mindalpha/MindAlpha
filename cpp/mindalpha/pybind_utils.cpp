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
#include <mindalpha/pybind_utils.h>
#include <mindalpha/stack_trace_utils.h>

namespace mindalpha
{

std::shared_ptr<pybind11::object> make_shared_pyobject(pybind11::object obj)
{
    std::shared_ptr<pybind11::object> obj_ptr(
        new pybind11::object(std::move(obj)),
        [](pybind11::object* ptr) {
            pybind11::gil_scoped_acquire gil;
            //pybind11::print("Python object", *ptr, "is deleted.");
            delete ptr;
        });
    return std::move(obj_ptr);
}

std::string serialize_pyobject(pybind11::object obj)
{
    if (obj.is_none())
        return {};
    pybind11::module base64 = pybind11::module::import("base64");
    pybind11::module cloudpickle = pybind11::module::import("cloudpickle");
    pybind11::bytes data = cloudpickle.attr("dumps")(obj);
    pybind11::str result = base64.attr("b64encode")(data).attr("decode")("ascii");
    return result;
}

pybind11::object deserialize_pyobject(const std::string& data)
{
    if (data.empty())
        return {};
    pybind11::module base64 = pybind11::module::import("base64");
    pybind11::module pickle = pybind11::module::import("pickle");
    pybind11::bytes buffer = base64.attr("b64decode")(pybind11::bytes(data));
    pybind11::object obj = pickle.attr("loads")(buffer);
    return obj;
}

void fixup_attributes(pybind11::object obj)
{
    pybind11::module compat = pybind11::module::import("mindalpha.compat");
    compat.attr("fixup_attributes")(obj);
}

pybind11::array make_numpy_array(SmartArray<uint8_t> data, DataType dtype)
{
    namespace py = pybind11;
    py::object obj = py::cast(data);
    py::buffer buf(obj);
    py::array arr(buf);
    py::array result;
    switch (dtype)
    {
#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u)                 \
    case mindalpha::DataType::u:                         \
        result = arr.attr("view")(#l);                   \
        break;                                           \
        /**/
    MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)
    }
    return result;
}

std::tuple<std::string_view, pybind11::bytes> make_string_object_tuple(pybind11::bytes obj)
{
    using namespace std::string_view_literals;
    const size_t length = PyBytes_Size(obj.ptr());
    const char* p = PyBytes_AsString(obj.ptr());
    if (p == nullptr || length == 0 || *p == '\0')
        return std::make_tuple("none"sv, std::move(obj));
    else
        return std::make_tuple(std::string_view{p, length}, std::move(obj));
}

std::tuple<std::string_view, pybind11::object> get_string_object_tuple(pybind11::object obj)
{
    using namespace std::string_view_literals;
    if (obj.is_none())
        return std::make_tuple("none"sv, std::move(obj));
    else if (pybind11::isinstance<pybind11::bytes>(obj))
        return make_string_object_tuple(obj.cast<pybind11::bytes>());
    else if (pybind11::isinstance<pybind11::str>(obj))
        return make_string_object_tuple(obj.attr("encode")("utf-8").cast<pybind11::bytes>());
    else
    {
        std::string serr;
        serr.append("None, bytes or str expected\n\n");
        serr.append(GetStackTrace());
        spdlog::error(serr);
        throw std::runtime_error(serr);
    }
}

}
