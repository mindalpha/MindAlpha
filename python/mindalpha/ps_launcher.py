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

class PSLauncher(object):
    def __init__(self):
        self._worker_count = None
        self._server_count = None
        self._job_name = None
        self._keep_session = None
        self._spark_log_level = None
        self._agent_attributes = None

    def _get_agent_class(self):
        raise NotImplementedError

    def _initialize_agent(self, agent):
        pass

    def launch_agent(self):
        import asyncio
        import pyspark
        # Use nest_asyncio to workaround the problem that ``asyncio.run()``
        # can not be called in Jupyter Notebook toplevel.
        import nest_asyncio
        nest_asyncio.apply()
        class_ = self._get_agent_class()
        builder = pyspark.sql.SparkSession.builder
        if self._job_name is not None:
            builder.appName(self._job_name)
        spark_session = builder.getOrCreate()
        try:
            if self._spark_log_level is not None:
                spark_context = spark_session.sparkContext
                spark_context.setLogLevel(self._spark_log_level)
            args = dict()
            args['worker_count'] = self._worker_count
            args['server_count'] = self._server_count
            args['agent_attributes'] = self._agent_attributes
            asyncio.run(class_._launch(args, spark_session, self))
        finally:
            if not self._keep_session:
                spark_session.stop()
