

import os
import sys

#  Add this directory to the path in GDB so import will work
path, filename = os.path.split(__file__)
sys.path.append(path)
import gdb
import gdb_helpers


import argparse
import numpy
import json
import pprint
import subprocess
import errno
import socket
import serial
import time
from datetime import datetime
from collections import OrderedDict

# IoT2 imports
import iot2_settings
import setup_result_collection
import iot2_measure_static_flash_and_ram
import iot2_run_experiments

#######################################################################
#                              GLOBALS                                #
#######################################################################


#######################################################################
#                              FUNCTIONS                              #
#######################################################################


#######################################################################
#                                 MAIN                                #
#######################################################################


def get_bench_heap_start():
    res = gdb.execute('info file', to_string=True)
    # Symbols... /.../../bench_name.elf".
    heap_start = 0
    bss_end = 0
    page_heap = 0 # to fix uvisor measurement
    res = res.split("\n")
    # we only care from line 4, the previous stuff is only info and nothing about the size
    i = 0
    for line in res:
        if i > 3 and len(line) > 1:
            size_info = line.split(" ")
            if size_info[4] == '.bss':
                bss_end = int(size_info[2], 16)
            elif size_info[4] == '.heap':
                heap_start = int(size_info[0].rsplit("\t")[1], 16)
            #elif size_info[4] == '.page_heap':
            #    page_heap = int(size_info[0].rsplit("\t")[1], 16)

        i += 1
    if page_heap != 0:
        return page_heap
    elif heap_start != 0:
        return heap_start
    else:
        return bss_end


def get_bench_res_configs():
    bench_metrics_result_dir = ""
    res = gdb.execute('info file', to_string=True)
    # Symbols... /.../../bench_name.elf".
    res = res.split("\n")[0].split("/")
    # read whole path except /bins/....elf
    for i in range(1, len(res)-2):
        bench_metrics_result_dir += '/' + res[i]
    bench_metrics_result_dir += '/metrics_results/'

    res = res[-1].split(".")[0]
    # to avoid any of mytool binary error, fix later [iot2-debug]
    if '--' in res:
        bench_name = res.split('--')[0]
    return bench_name, bench_metrics_result_dir


BIN_BENCH_NAME, BIN_BENCH_CONFIG_RES_DIR = get_bench_res_configs()

BIN_RESULT_FILE = BIN_BENCH_CONFIG_RES_DIR+ BIN_BENCH_NAME + iot2_settings.JSON_EXT

# to be set by get_bench_heap_start
BIN_HEAP_START = get_bench_heap_start()

gdb_helpers.cl()
# only enable this breakpoint if needed
if iot2_settings.BOARD_BUG_BKPT == 1:
    board_bug_bkpt = gdb_helpers.BOARD_BUG_BKPT("*0x1fff261e")

bp = gdb_helpers.EndBreakpoint("*iot2EndBenchmark", type=gdb.BP_BREAKPOINT)
bp.config(iot2_settings.IOT2_ITERATIONS, iot2_settings.STACK_TOP, BIN_HEAP_START, #iot2_settings.STACK_SIZE, #replaced stacksize with heap start
          iot2_settings.HEAP_START, iot2_settings.HEAP_SIZE,
          BIN_RESULT_FILE, iot2_settings.JSON_EXT) # use BIN_RESULT_FILE, others are redundant
gdb.execute('c')
#