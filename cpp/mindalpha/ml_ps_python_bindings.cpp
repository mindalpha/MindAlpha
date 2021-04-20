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

#include <mindalpha/io.h>
#include <mindalpha/ps_agent.h>
#include <mindalpha/ps_runner.h>
#include <mindalpha/pybind_utils.h>
#include <mindalpha/model_metric_buffer.h>
#include <mindalpha/ml_ps_python_bindings.h>
#include <mindalpha/tensor_store_python_bindings.h>
#include <mindalpha/feature_extraction_python_bindings.h>

namespace py = pybind11;

PYBIND11_MODULE(_mindalpha, m)
{
    py::enum_<mindalpha::NodeRole>(m, "NodeRole")
        .value("Coordinator", mindalpha::NodeRole::Coordinator)
        .value("Server", mindalpha::NodeRole::Server)
        .value("Worker", mindalpha::NodeRole::Worker)
        ;

    py::class_<mindalpha::NodeInfo>(m, "NodeInfo")
        .def_property("role", &mindalpha::NodeInfo::GetRole,
                              &mindalpha::NodeInfo::SetRole)
        .def_property("node_id", &mindalpha::NodeInfo::GetNodeId,
                                 &mindalpha::NodeInfo::SetNodeId)
        .def_property("host_name", &mindalpha::NodeInfo::GetHostName,
                                   &mindalpha::NodeInfo::SetHostName)
        .def_property("port", &mindalpha::NodeInfo::GetPort,
                              &mindalpha::NodeInfo::SetPort)
        .def_property_readonly("address", &mindalpha::NodeInfo::GetAddress)
        .def("__repr__", &mindalpha::NodeInfo::ToString)
        .def("__str__", &mindalpha::NodeInfo::ToShortString)
        ;

    py::class_<mindalpha::ActorConfig, std::shared_ptr<mindalpha::ActorConfig>>(m, "ActorConfig")
        .def(py::init<>())
        .def_property("agent_creator", &mindalpha::ActorConfig::GetAgentCreator,
                                       [](mindalpha::ActorConfig& self, py::object creator)
                                       {
                                           auto func = mindalpha::make_shared_pyobject(creator);
                                           self.SetAgentCreator([func] {
                                               py::gil_scoped_acquire gil;
                                               py::object obj = (*func)();
                                               return mindalpha::extract_shared_pyobject<mindalpha::PSAgent>(obj);
                                           });
                                       })
        .def_property("agent_ready_callback", &mindalpha::ActorConfig::GetAgentReadyCallback,
                                              [](mindalpha::ActorConfig& self, py::object cb)
                                              {
                                                  auto func = mindalpha::make_shared_pyobject(cb);
                                                  self.SetAgentReadyCallback([func](std::shared_ptr<mindalpha::PSAgent> agent) {
                                                      py::gil_scoped_acquire gil;
                                                      (*func)(agent);
                                                  });
                                              })
        .def_property("transport_type", &mindalpha::ActorConfig::GetTransportType,
                                        &mindalpha::ActorConfig::SetTransportType)
        .def_property("is_local_mode", &mindalpha::ActorConfig::IsLocalMode,
                                       &mindalpha::ActorConfig::SetIsLocalMode)
        .def_property("use_kubernetes", &mindalpha::ActorConfig::UseKubernetes,
                                        &mindalpha::ActorConfig::SetUseKubernetes)
        .def_property("root_uri", &mindalpha::ActorConfig::GetRootUri,
                                  &mindalpha::ActorConfig::SetRootUri)
        .def_property("root_port", &mindalpha::ActorConfig::GetRootPort,
                                   &mindalpha::ActorConfig::SetRootPort)
        .def_property("node_uri", &mindalpha::ActorConfig::GetNodeUri,
                                  &mindalpha::ActorConfig::SetNodeUri)
        .def_property("node_interface", &mindalpha::ActorConfig::GetNodeInterface,
                                        &mindalpha::ActorConfig::SetNodeInterface)
        .def_property("node_role", &mindalpha::ActorConfig::GetNodeRole,
                                   &mindalpha::ActorConfig::SetNodeRole)
        .def_property("node_port", &mindalpha::ActorConfig::GetNodePort,
                                   &mindalpha::ActorConfig::SetNodePort)
        .def_property("this_node_info", [](const mindalpha::ActorConfig& self)
                                        { return self.GetThisNodeInfo(); },
                                        &mindalpha::ActorConfig::SetThisNodeInfo)
        .def_property_readonly("is_coordinator", &mindalpha::ActorConfig::IsCoordinator)
        .def_property_readonly("is_server", &mindalpha::ActorConfig::IsServer)
        .def_property_readonly("is_worker", &mindalpha::ActorConfig::IsWorker)
        .def_property("bind_retry", &mindalpha::ActorConfig::GetBindRetry,
                                    &mindalpha::ActorConfig::SetBindRetry)
        .def_property("heartbeat_interval", &mindalpha::ActorConfig::GetHeartbeatInterval,
                                            &mindalpha::ActorConfig::SetHeartbeatInterval)
        .def_property("heartbeat_timeout", &mindalpha::ActorConfig::GetHeartbeatTimeout,
                                           &mindalpha::ActorConfig::SetHeartbeatTimeout)
        .def_property("is_message_dumping_enabled", &mindalpha::ActorConfig::IsMessageDumpingEnabled,
                                                    &mindalpha::ActorConfig::SetIsMessageDumpingEnabled)
        .def_property("is_resending_enabled", &mindalpha::ActorConfig::IsResendingEnabled,
                                              &mindalpha::ActorConfig::SetIsResendingEnabled)
        .def_property("resending_timeout", &mindalpha::ActorConfig::GetResendingTimeout,
                                           &mindalpha::ActorConfig::SetResendingTimeout)
        .def_property("resending_retry", &mindalpha::ActorConfig::GetResendingRetry,
                                         &mindalpha::ActorConfig::SetResendingRetry)
        .def_property("drop_rate", &mindalpha::ActorConfig::GetDropRate,
                                   &mindalpha::ActorConfig::SetDropRate)
        .def_property("server_count", &mindalpha::ActorConfig::GetServerCount,
                                      &mindalpha::ActorConfig::SetServerCount)
        .def_property("worker_count", &mindalpha::ActorConfig::GetWorkerCount,
                                      &mindalpha::ActorConfig::SetWorkerCount)
        .def("copy", &mindalpha::ActorConfig::Copy)
        ;

    py::class_<mindalpha::PSRunner>(m, "PSRunner")
        .def_static("run_ps", &mindalpha::PSRunner::RunPS, py::call_guard<py::gil_scoped_release>())
        ;

    py::class_<mindalpha::SmartArray<uint8_t>>(m, "SmartArray", py::buffer_protocol())
        .def_buffer([](mindalpha::SmartArray<uint8_t>& sa) {
                       return py::buffer_info(
                           sa.data(),                                /* Pointer to buffer */
                           sizeof(uint8_t),                          /* Size of one scalar */
                           py::format_descriptor<uint8_t>::format(), /* Python struct-style format descriptor */
                           1,                                        /* Number of dimensions */
                           { sa.size() },                            /* Buffer dimensions */
                           { sizeof(uint8_t) });                     /* Strides (in bytes) for each index */
                   });
        ;

    py::class_<mindalpha::Message, std::shared_ptr<mindalpha::Message>>(m, "Message")
        .def(py::init<>())
        .def_property_readonly("message_id", [](const mindalpha::Message& self)
                                             { return self.GetMessageMeta().GetMessageId(); })
        .def_property("sender", [](const mindalpha::Message& self)
                                { return self.GetMessageMeta().GetSender(); },
                                [](mindalpha::Message& self, int value)
                                { self.GetMessageMeta().SetSender(value); })
        .def_property("receiver", [](const mindalpha::Message& self)
                                  { return self.GetMessageMeta().GetReceiver(); },
                                  [](mindalpha::Message& self, int value)
                                  { self.GetMessageMeta().SetReceiver(value); })
        .def_property("is_request", [](const mindalpha::Message& self)
                                    { return self.GetMessageMeta().IsRequest(); },
                                    [](mindalpha::Message& self, bool value)
                                    { self.GetMessageMeta().SetIsRequest(value); })
        .def_property("is_response", [](const mindalpha::Message& self)
                                     { return !self.GetMessageMeta().IsRequest(); },
                                     [](mindalpha::Message& self, bool value)
                                     { self.GetMessageMeta().SetIsRequest(!value); })
        .def_property("is_exception", [](const mindalpha::Message& self)
                                      { return self.GetMessageMeta().IsException(); },
                                      [](mindalpha::Message& self, bool value)
                                      { self.GetMessageMeta().SetIsException(value); })
        .def_property("body", [](const mindalpha::Message& self)
                              { return self.GetMessageMeta().GetBody(); },
                              [](mindalpha::Message& self, std::string value)
                              { self.GetMessageMeta().SetBody(std::move(value)); })
        .def_property_readonly("slice_count", [](const mindalpha::Message& self)
                                              { return self.GetSlices().size(); })
        .def("clear_slices", [](mindalpha::Message& self)
                             { self.ClearSlicesAndDataTypes(); })
        .def("get_slice", [](mindalpha::Message& self, size_t index)
                          {
                              mindalpha::SmartArray<uint8_t> slice = self.GetSlice(index);
                              const mindalpha::MessageMeta& meta = self.GetMessageMeta();
                              const mindalpha::DataType type = meta.GetSliceDataTypes().at(index);
                              return mindalpha::make_numpy_array(slice, type);
                          })
#undef MINDALPHA_DATA_TYPE_DEF
#define MINDALPHA_DATA_TYPE_DEF(t, l, u)                                                                    \
        .def("add_slice", [](mindalpha::Message& self, py::array_t<t> arr)                                  \
                          {                                                                                 \
                              auto obj = mindalpha::make_shared_pyobject(arr);                              \
                              t* data = const_cast<t*>(arr.data(0));                                        \
                              auto sa = mindalpha::SmartArray<t>::Create(data, arr.size(), [obj](t*) { });  \
                              self.AddTypedSlice(sa);                                                       \
                          })                                                                                \
                          /**/
        MINDALPHA_DATA_TYPES(MINDALPHA_DATA_TYPE_DEF)
        .def("copy", &mindalpha::Message::Copy)
        .def("__str__", &mindalpha::Message::ToString)
        ;

    py::class_<mindalpha::PSAgent, mindalpha::PyPSAgent<>, std::shared_ptr<mindalpha::PSAgent>>(m, "PSAgent")
        .def(py::init<>())
        .def("run", &mindalpha::PSAgent::Run)
        .def("handle_request", &mindalpha::PSAgent::HandleRequest)
        .def("finalize", &mindalpha::PSAgent::Finalize)
        .def_property_readonly("is_coordinator", &mindalpha::PSAgent::IsCoordinator)
        .def_property_readonly("is_server", &mindalpha::PSAgent::IsServer)
        .def_property_readonly("is_worker", &mindalpha::PSAgent::IsWorker)
        .def_property_readonly("server_count", &mindalpha::PSAgent::GetServerCount)
        .def_property_readonly("worker_count", &mindalpha::PSAgent::GetWorkerCount)
        .def_property_readonly("rank", &mindalpha::PSAgent::GetAgentRank)
        .def("barrier", &mindalpha::PSAgent::Barrier, py::call_guard<py::gil_scoped_release>())
        // This adds an overload to default to setting up a barrier among all workers.
        .def("barrier", [](mindalpha::PSAgent& self)
                        {
                            py::gil_scoped_release gil;
                            self.Barrier(mindalpha::WorkerGroup);
                        })
        .def("shutdown", &mindalpha::PSAgent::Shutdown)
        .def("send_request", [](mindalpha::PSAgent& self,
                                mindalpha::PSMessage req,
                                py::object cb)
                             {
                                 auto func = mindalpha::make_shared_pyobject(cb);
                                 py::gil_scoped_release gil;
                                 self.SendRequest(req, [func](mindalpha::PSMessage req, mindalpha::PSMessage res)
                                 {
                                     py::gil_scoped_acquire gil;
                                     (*func)(req, res);
                                 });
                             })
        .def("send_all_requests", [](mindalpha::PSAgent& self,
                                     py::object reqs,
                                     py::object cb)
                                  {
                                      auto func = mindalpha::make_shared_pyobject(cb);
                                      std::vector<mindalpha::PSMessage> requests = mindalpha::make_cpp_vector<mindalpha::PSMessage>(reqs);
                                      py::gil_scoped_release gil;
                                      self.SendAllRequests(std::move(requests), [func](std::vector<mindalpha::PSMessage> reqs,
                                                                                       std::vector<mindalpha::PSMessage> ress)
                                      {
                                          py::gil_scoped_acquire gil;
                                          py::list requests = mindalpha::make_python_list(reqs);
                                          py::list responses = mindalpha::make_python_list(ress);
                                          (*func)(requests, responses);
                                      });
                                  })
        .def("broadcast_request", [](mindalpha::PSAgent& self,
                                     mindalpha::PSMessage req,
                                     py::object cb)
                                  {
                                      auto func = mindalpha::make_shared_pyobject(cb);
                                      py::gil_scoped_release gil;
                                      self.BroadcastRequest(req, [func](mindalpha::PSMessage req, std::vector<mindalpha::PSMessage> ress)
                                      {
                                          py::gil_scoped_acquire gil;
                                          py::list responses = mindalpha::make_python_list(ress);
                                          (*func)(req, responses);
                                      });
                                  })
        .def("send_response", &mindalpha::PSAgent::SendResponse)
        .def("__str__", &mindalpha::PSAgent::ToString)
        ;

    py::class_<mindalpha::ModelMetricBuffer>(m, "ModelMetricBuffer")
        .def("update_buffer", &mindalpha::ModelMetricBuffer::UpdateBuffer)
        .def("compute_auc", &mindalpha::ModelMetricBuffer::ComputeAUC)
        ;

    m.def("stream_write_all", [](const std::string& url, py::bytes data)
                              {
                                  char* buffer;
                                  ssize_t length;
                                  if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buffer, &length))
                                      py::pybind11_fail("Unable to extract bytes contents!");
                                  mindalpha::StreamWriteAll(url, buffer, length);
                              })
     .def("stream_write_all", [](const std::string& url, py::array data)
                              {
                                  const char* buffer = static_cast<const char*>(data.data());
                                  const size_t length = data.nbytes();
                                  mindalpha::StreamWriteAll(url, buffer, length);
                              })
     .def("stream_read_all", [](const std::string& url)
                             {
                                 std::string data = mindalpha::StreamReadAll(url);
                                 return py::bytes(data);
                             })
     .def("stream_read_all", [](const std::string& url, py::array data)
                             {
                                 char* buffer = static_cast<char*>(data.mutable_data());
                                 const size_t length = data.nbytes();
                                 mindalpha::StreamReadAll(url, buffer, length);
                             })
     .def("ensure_local_directory", &mindalpha::EnsureLocalDirectory)
     .def("get_mindalpha_version", []{ return _MINDALPHA_VERSION; })
     ;

    mindalpha::DefineTensorStoreBindings(m);
    mindalpha::DefineFeatureExtractionBindings(m);
}
