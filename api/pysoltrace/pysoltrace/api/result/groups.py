import ctypes
from pathlib import Path
from typing import Literal
import orjson

from pysoltrace import dot_h
from pysoltrace.api.utils import st_function
from pysoltrace.api.dll import context
from pysoltrace.soltrace_structs import group_result

############################################
# functions for grouped simulation results #
############################################
class groups(context):
    @st_function
    def num(self):
        pcount = ctypes.c_uint64()
        code = self._pdll.st_num_groups(self._pcxt, 
                                        ctypes.byref(pcount))
        return code, pcount.value

    def __len__(self):
        if not self._pdll or not self._pcxt:
            return 0
        return self.num()

    def __get_num(self):
        n = self.num()
        assert n > 0, 'No grouped results to report.'
        return n

    @st_function
    def __call__(self):
        n = self.__get_num()

        results = [group_result(n).ctype for _ in range(n)]

        code = self._pdll.st_get_grouped_results_data(self._pcxt,
                                                      (dot_h.args_group_result * n)(*results))
        # TODO: not setting values in struct properly
        return code, [_.value for _ in results]


