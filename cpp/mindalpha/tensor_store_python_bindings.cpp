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

#include <mindalpha/ps_agent.h>
#include <mindalpha/dense_tensor.h>
#include <mindalpha/sparse_tensor.h>
#include <mindalpha/pybind_utils.h>
#include <mindalpha/tensor_store_python_bindings.h>

namespace py = pybind11;

namespace mindalpha
{

void DefineTensorStoreBindings(pybind11::module& m)
{
    py::class_<mindalpha::DenseTensor>(m, "DenseTensor")
        .def(py::init<>())
        .def_property("name", [](const mindalpha::DenseTensor& self)
                              { return self.GetMeta().GetName(); },
                              [](mindalpha::DenseTensor& self, std::string value)
                              { self.GetMeta().SetName(std::move(value)); })
        .def_property("data_type", [](const mindalpha::DenseTensor& self)
                                   {
                                       const mindalpha::DataType t = self.GetMeta().GetDataType();
                                       return mindalpha::NullableDataTypeToString(t);
                                   },
                                   [](mindalpha::DenseTensor& self, const std::string& value)
                                   {
                                       const mindalpha::DataType t = mindalpha::NullableDataTypeFromString(value);
                                       self.GetMeta().SetDataType(t);
                                   })
        .def_property("data_shape", [](const mindalpha::DenseTensor& self)
                                    {
                                        const std::vector<size_t>& shape = self.GetMeta().GetDataShape();
                                        return mindalpha::make_python_tuple(shape);
                                    },
                                    [](mindalpha::DenseTensor& self, py::tuple value)
                                    {
                                        std::vector<size_t> shape = mindalpha::make_cpp_vector<size_t>(value);
                                        self.GetMeta().SetDataShape(std::move(shape));
                                    })
        .def_property("state_shape", [](const mindalpha::DenseTensor& self)
                                     {
                                         const std::vector<size_t>& shape = self.GetMeta().GetStateShape();
                                         return mindalpha::make_python_tuple(shape);
                                     },
                                     [](mindalpha::DenseTensor& self, py::tuple value)
                                     {
                                         std::vector<size_t> shape = mindalpha::make_cpp_vector<size_t>(value);
                                         self.GetMeta().SetStateShape(std::move(shape));
                                     })
        .def_property("initializer", [](const mindalpha::DenseTensor& self)
                                     {
                                         const std::string& data = self.GetMeta().GetInitializerAsData();
                                         return mindalpha::deserialize_pyobject(data);
                                     },
                                     [](mindalpha::DenseTensor& self, py::object value)
                                     {
                                         std::string data = mindalpha::serialize_pyobject(value);
                                         self.GetMeta().SetInitializerByData(std::move(data));
                                     })
        .def_property("updater", [](const mindalpha::DenseTensor& self)
                                 {
                                     const std::string& data = self.GetMeta().GetUpdaterAsData();
                                     return mindalpha::deserialize_pyobject(data);
                                 },
                                 [](mindalpha::DenseTensor& self, py::object value)
                                 {
                                     std::string data = mindalpha::serialize_pyobject(value);
                                     self.GetMeta().SetUpdaterByData(std::move(data));
                                 })
        .def_property("partition_count", [](const mindalpha::DenseTensor& self)
                                         { return self.GetMeta().GetPartitionCount(); },
                                         [](mindalpha::DenseTensor& self, int value)
                                         { self.GetMeta().SetPartitionCount(value); })
        .def_property("agent", &mindalpha::DenseTensor::GetAgent,
                               &mindalpha::DenseTensor::SetAgent)
        .def("__str__", [](const mindalpha::DenseTensor& self)
                        { return self.GetMeta().ToString(); })
        .def("init", [](mindalpha::DenseTensor& self, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Init([func]()
                         {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        .def("dispose", [](mindalpha::DenseTensor& self, py::object cb)
                        {
                            auto func = mindalpha::make_shared_pyobject(cb);
                            py::gil_scoped_release gil;
                            self.Dispose([func]()
                            {
                                py::gil_scoped_acquire gil;
                                (*func)();
                            });
                        })
        .def("push", [](mindalpha::DenseTensor& self, py::array in, py::object cb, bool is_value, bool is_state)
                     {
                         auto in_obj = mindalpha::make_shared_pyobject(in);
                         void* in_data_ptr = const_cast<void*>(in.data(0));
                         uint8_t* in_data = static_cast<uint8_t*>(in_data_ptr);
                         auto in_array = mindalpha::SmartArray<uint8_t>::Create(in_data, in.nbytes(), [in_obj](uint8_t*) { });
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Push(in_array, [func]()
                         {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         }, is_value, is_state);
                     })
        .def("pull", [](mindalpha::DenseTensor& self, py::object cb, bool is_state)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Pull([func, &self](mindalpha::SmartArray<uint8_t> out)
                         {
                             py::gil_scoped_acquire gil;
                             mindalpha::DataType type = self.GetMeta().GetDataType();
                             py::object out_arr = mindalpha::make_numpy_array(out, type);
                             py::tuple shape = mindalpha::make_python_tuple(self.GetMeta().GetDataShape());
                             out_arr = out_arr.attr("reshape")(shape);
                             (*func)(out_arr);
                         }, is_state);
                     })
        .def("load", [](mindalpha::DenseTensor& self,  const std::string& dir_path, py::object cb, bool keep_meta)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Load(dir_path ,[func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         }, keep_meta);
                     })
        .def("save", [](mindalpha::DenseTensor& self,  const std::string& dir_path, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Save(dir_path, [func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        ;

    py::class_<mindalpha::SparseTensor>(m, "SparseTensor")
        .def(py::init<>())
        .def_property("name", [](const mindalpha::SparseTensor& self)
                              { return self.GetMeta().GetName(); },
                              [](mindalpha::SparseTensor& self, std::string value)
                              { self.GetMeta().SetName(std::move(value)); })
        .def_property("data_type", [](const mindalpha::SparseTensor& self)
                                   {
                                       const mindalpha::DataType t = self.GetMeta().GetDataType();
                                       return mindalpha::NullableDataTypeToString(t);
                                   },
                                   [](mindalpha::SparseTensor& self, const std::string& value)
                                   {
                                       const mindalpha::DataType t = mindalpha::NullableDataTypeFromString(value);
                                       self.GetMeta().SetDataType(t);
                                   })
        .def_property("slice_data_shape", [](const mindalpha::SparseTensor& self)
                                          {
                                              const std::vector<size_t>& shape = self.GetMeta().GetSliceDataShape();
                                              return mindalpha::make_python_tuple(shape);
                                          },
                                          [](mindalpha::SparseTensor& self, py::tuple value)
                                          {
                                              std::vector<size_t> shape = mindalpha::make_cpp_vector<size_t>(value);
                                              self.GetMeta().SetSliceDataShape(std::move(shape));
                                          })
        .def_property("slice_state_shape", [](const mindalpha::SparseTensor& self)
                                           {
                                               const std::vector<size_t>& shape = self.GetMeta().GetSliceStateShape();
                                               return mindalpha::make_python_tuple(shape);
                                           },
                                           [](mindalpha::SparseTensor& self, py::tuple value)
                                           {
                                               std::vector<size_t> shape = mindalpha::make_cpp_vector<size_t>(value);
                                               self.GetMeta().SetSliceStateShape(std::move(shape));
                                           })
        .def_property("initializer", [](const mindalpha::SparseTensor& self)
                                     {
                                         const std::string& data = self.GetMeta().GetInitializerAsData();
                                         return mindalpha::deserialize_pyobject(data);
                                     },
                                     [](mindalpha::SparseTensor& self, py::object value)
                                     {
                                         std::string data = mindalpha::serialize_pyobject(value);
                                         self.GetMeta().SetInitializerByData(std::move(data));
                                     })
        .def_property("updater", [](const mindalpha::SparseTensor& self)
                                 {
                                     const std::string& data = self.GetMeta().GetUpdaterAsData();
                                     return mindalpha::deserialize_pyobject(data);
                                 },
                                 [](mindalpha::SparseTensor& self, py::object value)
                                 {
                                     std::string data = mindalpha::serialize_pyobject(value);
                                     self.GetMeta().SetUpdaterByData(std::move(data));
                                 })
        .def_property("partition_count", [](const mindalpha::SparseTensor& self)
                                         { return self.GetMeta().GetPartitionCount(); },
                                         [](mindalpha::SparseTensor& self, int value)
                                         { self.GetMeta().SetPartitionCount(value); })
        .def_property("agent", &mindalpha::SparseTensor::GetAgent,
                               &mindalpha::SparseTensor::SetAgent)
        .def("__str__", [](const mindalpha::SparseTensor& self)
                        { return self.GetMeta().ToString(); })
        .def("init", [](mindalpha::SparseTensor& self, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Init([func]()
                         {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        .def("dispose", [](mindalpha::SparseTensor& self, py::object cb)
                        {
                            auto func = mindalpha::make_shared_pyobject(cb);
                            py::gil_scoped_release gil;
                            self.Dispose([func]()
                            {
                                py::gil_scoped_acquire gil;
                                (*func)();
                            });
                        })
        .def("clear", [](mindalpha::SparseTensor& self, py::object cb)
                      {
                          auto func = mindalpha::make_shared_pyobject(cb);
                          py::gil_scoped_release gil;
                          self.Clear([func]()
                          {
                              py::gil_scoped_acquire gil;
                              (*func)();
                          });
                      })
        .def("push", [](mindalpha::SparseTensor& self, py::array keys, py::array in, py::object cb, bool is_value)
                     {
                         auto keys_obj = mindalpha::make_shared_pyobject(keys);
                         auto in_obj = mindalpha::make_shared_pyobject(in);
                         void* keys_data_ptr = const_cast<void*>(keys.data(0));
                         void* in_data_ptr = const_cast<void*>(in.data(0));
                         uint8_t* keys_data = static_cast<uint8_t*>(keys_data_ptr);
                         uint8_t* in_data = static_cast<uint8_t*>(in_data_ptr);
                         auto keys_array = mindalpha::SmartArray<uint8_t>::Create(keys_data, keys.nbytes(), [keys_obj](uint8_t*) { });
                         auto in_array = mindalpha::SmartArray<uint8_t>::Create(in_data, in.nbytes(), [in_obj](uint8_t*) { });
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Push(keys_array, in_array, [func]()
                         {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         }, is_value);
                     })
        .def("pull", [](mindalpha::SparseTensor& self, py::array keys, py::object cb, bool read_only)
                     {
                         auto keys_obj = mindalpha::make_shared_pyobject(keys);
                         void* keys_data_ptr = const_cast<void*>(keys.data(0));
                         uint8_t* keys_data = static_cast<uint8_t*>(keys_data_ptr);
                         auto keys_array = mindalpha::SmartArray<uint8_t>::Create(keys_data, keys.nbytes(), [keys_obj](uint8_t*) { });
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Pull(keys_array, [func, &self](mindalpha::SmartArray<uint8_t> out)
                         {
                             py::gil_scoped_acquire gil;
                             mindalpha::DataType type = self.GetMeta().GetDataType();
                             py::object out_arr = mindalpha::make_numpy_array(out, type);
                             const std::vector<size_t>& slice_shape = self.GetMeta().GetSliceDataShape();
                             py::tuple shape(1 + slice_shape.size());
                             shape[0] = -1;
                             for (size_t i = 0; i < slice_shape.size(); i++)
                                 shape[1 + i] = static_cast<int64_t>(slice_shape.at(i));
                             out_arr = out_arr.attr("reshape")(shape);
                             (*func)(out_arr);
                         }, read_only);
                     })
        .def("load", [](mindalpha::SparseTensor& self, const std::string& dir_path, py::object cb, bool keep_meta)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Load(dir_path, [func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         }, keep_meta);
                     })
        .def("save", [](mindalpha::SparseTensor& self,  const std::string& dir_path, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Save(dir_path, [func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        .def("export", [](mindalpha::SparseTensor& self,  const std::string& dir_path, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.Export(dir_path, [func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        .def("import_from", [](mindalpha::SparseTensor& self, const std::string& meta_file_path, py::object cb,
                               bool data_only, bool skip_existing)
                            {
                                auto func = mindalpha::make_shared_pyobject(cb);
                                py::gil_scoped_release gil;
                                self.ImportFrom(meta_file_path, [func]() {
                                    py::gil_scoped_acquire gil;
                                    (*func)();
                                }, data_only, skip_existing);
                            })
        .def("prune_small", [](mindalpha::SparseTensor& self,  double epsilon, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.PruneSmall(epsilon, [func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        .def("prune_old", [](mindalpha::SparseTensor& self,  int max_age, py::object cb)
                     {
                         auto func = mindalpha::make_shared_pyobject(cb);
                         py::gil_scoped_release gil;
                         self.PruneOld(max_age, [func]() {
                             py::gil_scoped_acquire gil;
                             (*func)();
                         });
                     })
        ;

    py::class_<mindalpha::PSDefaultAgent,
               mindalpha::PyPSDefaultAgent<>,
               std::shared_ptr<mindalpha::PSDefaultAgent>,
               mindalpha::PSAgent>
        (m, "PSDefaultAgent")
        .def(py::init<>())
        .def_property("py_agent", &mindalpha::PSDefaultAgent::GetPyAgent, &mindalpha::PSDefaultAgent::SetPyAgent)
        .def("run", &mindalpha::PSDefaultAgent::Run)
        .def("handle_request", &mindalpha::PSDefaultAgent::HandleRequest)
        .def("finalize", &mindalpha::PSDefaultAgent::Finalize)
        ;
}

}
