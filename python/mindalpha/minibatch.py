import numpy as np
class Minibatch(object):
    def __init__(self, df):
        ndarrays = df.to_numpy()
        self._column_values = ndarrays.T
        self._column_names = [col for  col in df.columns]
        self._column_names_map = {col:idx for idx, col in enumerate(df.columns)}
    @property
    def column_values(self):
        return self._column_values
    @property
    def column_names(self):
        return self._column_names
    def get_column_idx(self, column_name):
        return self._column_names_map.get(column_name)
