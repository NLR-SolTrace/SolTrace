#!/usr/bin/env python3

import re, sys
from pathlib import Path

current_dir = Path().absolute()
stapi_v2_h  = current_dir / 'stapi_v2.h'

RE_STAPI_V2_FUNC = re.compile(r'STAPI_V2 st_return_t (?P<func>[A-Za-z_]\w*)\((?P<args>[\w\s,*=_]*)\) noexcept;')

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

    for match in RE_STAPI_V2_FUNC.finditer(stapi_v2_h_str):
        print('\n', match.group('func'))
        # print(match.group('args'))
        # cleaned_args = re.sub(r"\s+", " ", match.group('args')).split(',')
        # for type_name in cleaned_args:
        #     print(type_name.split())
        clean_args = clean_stapi_v2_args(match.group('args'))
        print(clean_args)
        print(make_args_struct(match.group('func'), clean_args))