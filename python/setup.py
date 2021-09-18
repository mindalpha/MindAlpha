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

#
# This script derives from the following link:
#
#   https://stackoverflow.com/questions/42585210/extending-setuptools-extension-to-use-cmake-in-setup-py
#

from setuptools import setup
from setuptools import Extension
from setuptools.command.build_ext import build_ext

class MindAlphaExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])

class mindalpha_build_ext(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_mindalpha(ext)

    def get_mindalpha_so_path(self):
        import os
        key = '_MINDALPHA_SO'
        path = os.environ.get(key)
        if path is None:
            message = "environment variable %r is not set; " % key
            message += "can not find path of '_mindalpha.so'"
            raise RuntimeError(message)
        if not os.path.isfile(path):
            message = "'_mindalpha.so' is not found at %r" % path
            raise RuntimeError(message)
        return path

    def build_mindalpha(self, ext):
        import shutil
        mindalpha_so_path = self.get_mindalpha_so_path()
        ext_so_path = self.get_ext_fullpath(ext.name)
        shutil.copy(mindalpha_so_path, ext_so_path)

def get_mindalpha_version():
    import os
    key = '_MINDALPHA_VERSION'
    mindalpha_version = os.environ.get(key)
    if mindalpha_version is None:
        message = "environment variable %r is not set; " % key
        message += "can not get MindAlpha wheel version"
        raise RuntimeError(message)
    return mindalpha_version

setup(name='mindalpha',
      version=get_mindalpha_version(),
      description="MindAlpha machine learning platform.",
      packages=['mindalpha', 'mindalpha.compat', 'mindalpha.compat.ps', 'ps'],
      ext_modules=[MindAlphaExtension('mindalpha/_mindalpha')],
      cmdclass={ 'build_ext': mindalpha_build_ext },
      install_requires=['numpy>=1.20.1',
                        'pandas>=1.2.3',
                        'nest_asyncio>=1.5.1',
                        'cloudpickle>=1.6.0',
                        'pyarrow>=3.0.0',
                        'PyYAML>=5.3.1',
                        'boto3>=1.17.41',
                        'python-consul>=1.1.0',
                        'findspark>=1.4.2'])
