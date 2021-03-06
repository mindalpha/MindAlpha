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

from ._mindalpha import NodeRole
from ._mindalpha import ActorConfig
from ._mindalpha import PSRunner

from .embedding import EmbeddingSumConcat
from .embedding import EmbeddingRangeSum
from .embedding import EmbeddingLookup

from .cast import Cast

from .initializer import TensorInitializer
from .initializer import DefaultTensorInitializer
from .initializer import ZeroTensorInitializer
from .initializer import OneTensorInitializer
from .initializer import NormalTensorInitializer
from .initializer import XavierTensorInitializer

from .updater import TensorUpdater
from .updater import NoOpUpdater
from .updater import SGDTensorUpdater
from .updater import AdaGradTensorUpdater
from .updater import AdamTensorUpdater
from .updater import FTRLTensorUpdater
from .updater import EMATensorUpdater

from .agent import Agent
from .model import Model
from .model import SparseModel
from .metric import ModelMetric
from .distributed_trainer import DistributedTrainer
from .experiment import Experiment

try:
    import pyspark
except ImportError:
    # Use findspark to simplify running job locally.
    try:
        import findspark
        findspark.init()
    except:
        pass

try:
    import pyspark
except ImportError:
    pass
else:
    # PySpark may not be available at this point,
    # we import the classes in estimator only when
    # PySpark is ready.

    from .estimator import PyTorchAgent
    from .estimator import PyTorchLauncher
    from .estimator import PyTorchModel
    from .estimator import PyTorchEstimator

    from .two_tower_ranking import TwoTowerRankingModule
    from .two_tower_ranking import TwoTowerRankingAgent
    from .two_tower_ranking import TwoTowerRankingLauncher
    from .two_tower_ranking import TwoTowerRankingModel
    from .two_tower_ranking import TwoTowerRankingEstimator

    from .swing_retrieval import SwingModel
    from .swing_retrieval import SwingEstimator

try:
    import pyspark
    import faiss
except ImportError:
    pass
else:
    from .two_tower_retrieval import TwoTowerRetrievalModule
    from .two_tower_retrieval import FaissIndexBuildingAgent
    from .two_tower_retrieval import FaissIndexRetrievalAgent
    from .two_tower_retrieval import TwoTowerRetrievalModel
    from .two_tower_retrieval import TwoTowerRetrievalEstimator

from ._mindalpha import get_mindalpha_version
__version__ = get_mindalpha_version()
del get_mindalpha_version

from . import nn
from . import input
from . import output
from . import spark
from . import patching_pickle
from . import demo

patching_pickle._patch_lookup_module_and_qualname()
patching_pickle._patch_getsourcelines()
