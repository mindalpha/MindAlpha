import numpy as np
class Minibatch(object):
    def __init__(self, dataframe=None, columns=None):
        if dataframe is None:
            self._column_values = [col.values for col in columns]
            self._column_names = None
            self._column_names_map = None
        else:
            ndarrays = dataframe.to_numpy()
            self._column_values = ndarrays.T
            self._column_names = [col for  col in dataframe.columns]
            self._column_names_map = {col:idx for idx, col in enumerate(dataframe.columns)}

    @property
    def column_values(self):
        return self._column_values
    @property
    def column_names(self):
        return self._column_names
    def get_column_idx(self, column_name):
        return self._column_names_map.get(column_name)
