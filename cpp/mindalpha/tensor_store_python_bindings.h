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

#include <pybind11/pybind11.h>
#include <mindalpha/ml_ps_python_bindings.h>
#include <mindalpha/ps_default_agent.h>

namespace mindalpha
{

template<class PSAgentBase = mindalpha::PSDefaultAgent>
class PyPSDefaultAgent : public PyPSAgent<PSAgentBase> { };

void DefineTensorStoreBindings(pybind11::module& m);

}
