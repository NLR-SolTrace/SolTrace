#!/usr/bin/env python3

from __future__ import annotations

import ctypes, re, sys
from pathlib import Path
from typing import Any, Dict, List, Literal, Optional, Tuple

current_dir = Path().absolute()
stapi_v2_h  = current_dir / 'stapi_v2.h'

_RE_STAPI_V2_FUNC = re.compile(r'STAPI_V2 st_return_t (?P<func>[A-Za-z_]\w*)\((?P<args>[\w\s,*=_]*)\) noexcept;')

_ENUM_RE = re.compile(
    r"(?:typedef\s+)?enum\s*(?:class\s+)?\s*(?:(?P<tag>[A-Za-z_]\w*)\s*)?"
    r"(?::\s*(?P<underlying>[A-Za-z_][\w\s]*?)\s*)?"
    r"\{(?P<body>[^}]*)\}\s*(?:(?P<name>[A-Za-z_]\w*)\s*)?;",
    re.DOTALL,
)

# ---------------------------------------------------------------------------
# ctypes type resolution
# ---------------------------------------------------------------------------

_BASE_CTYPES: Dict[str, Optional[type]] = {
    "void":               None,
    "char":               ctypes.c_char,
    "signed char":        ctypes.c_byte,
    "unsigned char":      ctypes.c_ubyte,
    "short":              ctypes.c_short,
    "short int":          ctypes.c_short,
    "unsigned short":     ctypes.c_ushort,
    "unsigned short int": ctypes.c_ushort,
    "int":                ctypes.c_int,
    "unsigned int":       ctypes.c_uint,
    "unsigned":           ctypes.c_uint,
    "long":               ctypes.c_long,
    "unsigned long":      ctypes.c_ulong,
    "long long":          ctypes.c_longlong,
    "unsigned long long": ctypes.c_ulonglong,
    "float":              ctypes.c_float,
    "double":             ctypes.c_double,
    "long double":        ctypes.c_longdouble,
    "bool":               ctypes.c_bool,
    "_Bool":              ctypes.c_bool,
    "size_t":             ctypes.c_size_t,
    "ssize_t":            ctypes.c_ssize_t,
    "int8_t":             ctypes.c_int8,
    "uint8_t":            ctypes.c_uint8,
    "int16_t":            ctypes.c_int16,
    "uint16_t":           ctypes.c_uint16,
    "int32_t":            ctypes.c_int32,
    "uint32_t":           ctypes.c_uint32,
    "int64_t":            ctypes.c_int64,
    "uint64_t":           ctypes.c_uint64,
    "int_fast8_t":        ctypes.c_int8,
    "uint_fast8_t":       ctypes.c_uint8,
    "int_fast16_t":       ctypes.c_int64,
    "uint_fast16_t":      ctypes.c_uint64,
    "int_fast32_t":       ctypes.c_int64,
    "uint_fast32_t":      ctypes.c_uint64,
    "int_fast64_t":       ctypes.c_int64,
    "uint_fast64_t":      ctypes.c_uint64,
    "int_least8_t":       ctypes.c_int8,
    "uint_least8_t":      ctypes.c_uint8,
    "int_least16_t":      ctypes.c_int16,
    "uint_least16_t":     ctypes.c_uint16,
    "int_least32_t":      ctypes.c_int32,
    "uint_least32_t":     ctypes.c_uint32,
    "int_least64_t":      ctypes.c_int64,
    "uint_least64_t":     ctypes.c_uint64,
    "intmax_t":           ctypes.c_int64,
    "uintmax_t":          ctypes.c_uint64,
    "intptr_t":           ctypes.c_ssize_t,
    "uintptr_t":          ctypes.c_size_t,
    "ptrdiff_t":          ctypes.c_ssize_t,
    "wchar_t":            ctypes.c_wchar,
}


def clean_stapi_v2_args(args_match: str):
    cleaned_args = re.sub(r"\s+", " ", args_match).split(',')
    return [tuple(type_name.split(maxsplit = 1)) for type_name in cleaned_args]

def make_args_struct(func_name: str, args: list[tuple[str, str]]):
    # ignore context pointer bc implied 
    if args[0][0] == 'st_context_v2_t':
        args = args[1:]

    # only make struct if other args than context pointer
    if len(args) < 1: 
        print(func_name)
        return ''

    # function struct name
    STRUCT_NAME = 'args_' + func_name
    # first line of struct
    rt_str = 'typedef struct ' + STRUCT_NAME + ' {\n'
    # for printing nicely
    longest_arg_name = max([len(arg[0]) for arg in args])
    # fill body of struct
    for type_name in args:
        rt_str += f"{' ' * 4}{type_name[0]:<{longest_arg_name}} {type_name[1]};\n"
    # last line of struct
    rt_str += '} ' + STRUCT_NAME + ';\n'

    return rt_str

if __name__ == '__main__':
    if not stapi_v2_h.exists(): sys.exit(1)

    stapi_v2_h_f = open(stapi_v2_h)
    stapi_v2_h_str = stapi_v2_h_f.read()
    stapi_v2_h_f.close()

    # print(stapi_v2_h_str)

    for match in _RE_STAPI_V2_FUNC.finditer(stapi_v2_h_str):
        print('\n', match.group('func'))
        # print(match.group('args'))
        # cleaned_args = re.sub(r"\s+", " ", match.group('args')).split(',')
        # for type_name in cleaned_args:
        #     print(type_name.split())
        clean_args = clean_stapi_v2_args(match.group('args'))
        print(clean_args)
        print(make_args_struct(match.group('func'), clean_args))

    for match in _ENUM_RE.finditer(stapi_v2_h_str):
        print('\n', match.group('name'))
        print(match.group('underlying'))
        print(match.group('body'))
#         underlying
# body
# name