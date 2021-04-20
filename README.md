# MindAlpha

MindAlpha is a machine learning platform integrating PySpark, PyTorch
and a parameter server implementation. The platform contains native
support for sparse parameters, making it easy for users to develop
large-scale models. Together with MindAlpha Serving, the platform
provides a one-stop solution for data preprocessing, model training and
online prediction.

## Features

* Efficient IO with PySpark. Minibatches read by PySpark as pandas DataFrames
  can be feed directly to models.

* Similar API with PyTorch and Spark ML, users familar with PyTorch and
  PySpark can get started quickly.

* Wrap custom sparse layers as PyTorch modules, making them easy to use.
  Those sparse layers can contain billions of parameters.

* Models can be developed in Jupyter Notebook interactively and periodical
  model training can be scheduled by Airflow.

* The trained model can be exported via one method call and loaded by MindAlpha
  Serving for online prediction.

## Build

Use [docker/Dockerfile](docker/Dockerfile) to build a docker image
and launch a container with the image, then execute ``./compile.sh``
in the container.

## Tutorials

Two tutorials are given:

* [MindAlpha Getting Started](tutorials/mindalpha-getting-started.ipynb) introduces the basic API of MindAlpha briefly.
* [MindAlpha Tutorial](tutorials/mindalpha-tutorial.ipynb) shows how to use MindAlpha in production setting.
