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

from mindalpha import NodeRole as ActorRole
from mindalpha import ActorConfig
from mindalpha import PSRunner

from mindalpha import EmbeddingSumConcat
from mindalpha import EmbeddingRangeSum
from mindalpha import EmbeddingLookup

from mindalpha import TensorInitializer
from mindalpha import DefaultTensorInitializer
from mindalpha import ZeroTensorInitializer
from mindalpha import OneTensorInitializer
from mindalpha import NormalTensorInitializer
from mindalpha import XavierTensorInitializer

from mindalpha import TensorUpdater
from mindalpha import NoOpUpdater
from mindalpha import SGDTensorUpdater
from mindalpha import AdaGradTensorUpdater
from mindalpha import AdamTensorUpdater
from mindalpha import FTRLTensorUpdater
from mindalpha import EMATensorUpdater

from mindalpha import Agent
from mindalpha import Model
from mindalpha import SparseModel
from mindalpha import ModelMetric as ModelCriterion
from mindalpha import DistributedTrainer

try:
    import pyspark
except ImportError:
    pass
else:
    from mindalpha import PyTorchAgent
    from mindalpha import PyTorchLauncher
    from mindalpha import PyTorchModel
    from mindalpha import PyTorchEstimator

from mindalpha import __version__
from mindalpha import _mindalpha as _ps

from mindalpha import nn
from mindalpha import input
from mindalpha import spark

from mindalpha import initializer
from mindalpha import updater
