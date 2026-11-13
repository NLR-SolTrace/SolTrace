import ctypes
from pathlib import Path
from typing import Literal
import orjson

from pysoltrace import dot_h
from pysoltrace.api.utils import st_function
from pysoltrace.api.dll import context

# TODO: for when group results are implemented

##############################################################
# functions for simulation data management thru json strings #
##############################################################
class json(context):
    @st_function
    def dump(self, filename: str, precision: int = 12, indent: int = 4):
        return self._pdll.st_write_group_results_json(self._pcxt, filename.encode(), precision, indent)