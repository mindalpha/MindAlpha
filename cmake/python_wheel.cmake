#
# Copyright 2021 Mobvista
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

get_python_wheel_tag(python_wheel_tag)
set(wheel_file_name mindalpha-${project_version}-${python_wheel_tag}.whl)
message(STATUS "python_wheel_tag: ${python_wheel_tag}")
message(STATUS "wheel_file_name: ${wheel_file_name}")

set(python_files
    python/setup.py
    python/mindalpha/__init__.py
    python/mindalpha/initializer.py
    python/mindalpha/updater.py
    python/mindalpha/model.py
    python/mindalpha/distributed_trainer.py
    python/mindalpha/distributed_tensor.py
    python/mindalpha/agent.py
    python/mindalpha/metric.py
    python/mindalpha/loss_utils.py
    python/mindalpha/nn.py
    python/mindalpha/embedding.py
    python/mindalpha/cast.py
    python/mindalpha/input.py
    python/mindalpha/output.py
    python/mindalpha/url_utils.py
    python/mindalpha/s3_utils.py
    python/mindalpha/file_utils.py
    python/mindalpha/name_utils.py
    python/mindalpha/network_utils.py
    python/mindalpha/shell_utils.py
    python/mindalpha/stack_trace_utils.py
    python/mindalpha/ps_launcher.py
    python/mindalpha/job_utils.py
    python/mindalpha/estimator.py
    python/mindalpha/retrieval.py
    python/mindalpha/swing.py
    python/mindalpha/experiment.py
    python/mindalpha/spark.py
    python/mindalpha/patching_pickle.py
)
add_custom_command(OUTPUT ${wheel_file_name}
                   COMMAND env _MINDALPHA_SO=${PROJECT_BINARY_DIR}/_mindalpha.so
                               _MINDALPHA_VERSION=${project_version}
                           ${Python_EXECUTABLE} -m pip wheel ${PROJECT_SOURCE_DIR}/python
                   MAIN_DEPENDENCY python/setup.py
                   DEPENDS mindalpha_shared ${python_files})
add_custom_target(python_wheel ALL DEPENDS ${wheel_file_name})
