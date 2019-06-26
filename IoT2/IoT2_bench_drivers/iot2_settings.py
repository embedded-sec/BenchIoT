import os
import sys
import argparse
import numpy
import json
import subprocess
import errno

#######################################################################
#                              GLOBALS                                #
#######################################################################


CURR_PATH = os.path.dirname(os.path.realpath(__file__))
PROJ_ROOT_DIR = os.path.abspath(CURR_PATH+'/../../')
IoT2_EVAL_CONFIG_FILE = PROJ_ROOT_DIR + "/IoT2/IoT2-Eval-Config/evaluation-results-config.json"
IoT2_METRIC_CONFIG_FILE = PROJ_ROOT_DIR + "/IoT2/IoT2-Eval-Config/eval-metric-config.json"
IOT2_CONFIGS = {}
# values to be shared with other files to simplify collecting results
BENCHMARKS_ROOT_DIR = ""
BENCHMARKS_PATH = ""
BOARD_NAME = ""
IOT2_ITERATIONS = 1
RESULTS_DIR_PATH = "RESULTS_DIR"
BINS_PATH = ""
ANALYSIS_RESULTS_PATH = ""
METRICS_RESULTS_PATH = ""
BENCHMARKS_VERIFICATION_PATH = ""
RESULTS_TYPE_DIR = ""
OS_BENCHMARKS = 1

# file extensions
SIZE_EXT = '.size'
BIN_EXT = '.elf'
CSV_EXT = '.csv'
JSON_EXT = '.json'
TXT_EXT = ".txt"

BUILD_DIR = "BUILD_DIR"

# Used to check entering the command line option to run all the benchmarks
ALL = "all"
# Used to run only the list from the configuration file
MANUAL_BENCH_LIST = []

DEBUGGER = ""
GDB_SCRIPT = "iot2_run_benchmark.py"
BENCHMARK_END = ""
IOT2_SERIAL_PORT = ""

BOARD_IP = ""
BOARD_PORT = 0
PC_IP = ""

# global for bootloader file
BOOTLOADER_BIN_FILE = ""

# MAX value for cycle counter, used to calculate the total runtime cycles, this is equal to
# IOT2_CYCCNT_OVERFLOW_LIMIT in IoT2Lib.h
CYCLE_COUNTER_MAX = 4080218932


# metric names, these are defined here and match the names sent by iot2resultCollector to be able to produce
# csv files and plots from the json results file
IOBOUND_CYCLES = "IOBound_cycles"
ISR_CYCLES = "ISR_cycles"
INIT_CYCLES = "Init_cycles"
IOT2_OVERHEAD_CYCLES = "IoT2Overhead_cycles"
IOT2_THROUGHPUT = "IoT2throughput"
PENDSV_CYCLES = "PendSV_cycles"
PRIV_THREAD_CYCLES = "PrivThread_cycles"
PRIVILEGED_CYCLES = "Priv_cycles"
SVC_CYCLES = "SVC_cycles"
SLEEP_CYCLES = "Sleep_cycles"
SYSTICK_CYCLES = "SysTick_cycles"
TOTAL_RUNTIME_DICTIONARY = "total_runtime"
RUNTIME_NUM_OVERFLOWS = "num_overflows"
RUNTIME_CYCLES = "runtime"
TOTAL_RUNTIME_CYCLES = "total_runtime_cycles"

# dynamic memory metrics setup
STACK_TOP = 0
STACK_SIZE = 0
HEAP_START = 0
HEAP_SIZE = 0


# This variable controls whether to enable a breakpoint to
# handle a but specific to the boot sequence of a board
# set the option to 0 in JSON file to disable it
BOARD_BUG_BKPT = 0


# metrics variables
METRIC_CATAGORIES = {}
SECURITY_METRICS = []
PERFORMANCE_METRICS = []
FLASH_METRICS = []
RAM_METRICS = []
ENERGY_METRICS = []

# variables used to ease getting type of metrics
SECURITY_METRICS_TYPE = "SECURITY_METRICS"
PERFORMANCE_METRICS_TYPE = "PERFORMANCE_METRICS"
FLASH_METRICS_TYPE = "FLASH_METRICS"
RAM_METRICS_TYPE = "RAM_METRICS"
ENERGY_METRICS_TYPE = "ENERGY_METRICS"

# name for final results file
ALL_BENCHES_RESULT_FILE = "ALL_BENCHES_RESULT_FILE"


# ---------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------

MBED_CMD = "MBED_CMD"
MAKE_CMD = "MAKE_CMD"
CLEAN_CMD = "CLEAN_CMD"
ALL_STR = 'all'
BENCH_RESULTS_DIR = "BENCH_RESULTS_DIR"
BENCH_TYPE = 'BENCH_TYPE'
OS_RESULTS_DIR = 'OS_RESULTS_DIR'
BAREMETAL_RESULTS_DIR = 'BAREMETAL_RESULTS_DIR'
BINS_DIR = "BINS_DIR"
BENCHMARK_VERIFICATION_DIR = "BENCHMARK_VERIFICATION_DIR"
ANALYSIS_FILES_DIR = "ANALYSIS_FILES_DIR"
METRICS_RESULTS_DIR = "METRICS_RESULTS_DIR"
BENCH_CONFIG_RES_DIR = ""
BASELINE_DIR = "BASELINE_DIR"

BUILD_OPTIONS = {
    'mbed-os':{
        MBED_CMD: ['mbed','export', '-m',str(BOARD_NAME), '-i', 'GCC_ARM', '--profile', 'release'],
        CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1',
                   'CUSTOM_BIN_SUFFIX=--baseline',
                   'clean'],
        MAKE_CMD: ['make', '-j4', 'BOARD='+str(BOARD_NAME), 'IOT2_ALL=1', 'CUSTOM_BIN_SUFFIX=--baseline',
                   'iot2benchmark']
    },
    'mbed': {
        MBED_CMD: ['mbed', 'export', '-m',str(BOARD_NAME), '-i', 'GCC_ARM', '--profile', 'release'],
        CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'CUSTOM_BIN_SUFFIX=--baseline',
                   'clean'],
        MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1',
                   'CUSTOM_BIN_SUFFIX=--baseline',
                   'iot2benchmark']
    },
    'uvisor':{
        MBED_CMD: ['mbed', 'export', '-m',str(BOARD_NAME), '-i', 'GCC_ARM', '--profile', 'release'],
        CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_UVISOR=1',
                   'IOT2_CONFIG=3', 'CUSTOM_LINKER=uvisor', 'CUSTOM_BIN_SUFFIX=--uvisor', 'clean'],
        MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_UVISOR=1',
                   'IOT2_CONFIG=3', 'CUSTOM_LINKER=uvisor', 'CUSTOM_BIN_SUFFIX=--uvisor', 'iot2benchmark']
    }
}



#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def init_configs():
    global IOT2_CONFIGS, BENCHMARKS_PATH, IOT2_ITERATIONS, BOARD_NAME, \
        BINS_PATH, BENCHMARK_END, IOT2_SERIAL_PORT, BOARD_IP, BOARD_PORT, PC_IP, \
        RESULTS_DIR_PATH, METRICS_RESULTS_PATH, CYCLE_COUNTER_MAX, IOBOUND_CYCLES, \
        ISR_CYCLES, INIT_CYCLES, IOT2_OVERHEAD_CYCLES, IOT2_THROUGHPUT, PENDSV_CYCLES, \
        PRIV_THREAD_CYCLES, PRIVILEGED_CYCLES, SVC_CYCLES, SLEEP_CYCLES, SYSTICK_CYCLES, \
        TOTAL_RUNTIME_DICTIONARY, RUNTIME_NUM_OVERFLOWS, RUNTIME_CYCLES, TOTAL_RUNTIME_CYCLES, \
        BOOTLOADER_BIN_FILE, BENCHMARKS_VERIFICATION_PATH, BOARD_BUG_BKPT, \
        SECURITY_METRICS, PERFORMANCE_METRICS, FLASH_METRICS, RAM_METRICS, ENERGY_METRICS, \
        RESULTS_TYPE_DIR, METRIC_CATAGORIES, SECURITY_METRICS_TYPE, PERFORMANCE_METRICS_TYPE, \
        FLASH_METRICS_TYPE, RAM_METRICS_TYPE, ENERGY_METRICS_TYPE, ANALYSIS_RESULTS_PATH, \
        ALL_BENCHES_RESULT_FILE, MANUAL_BENCH_LIST, STACK_TOP, STACK_SIZE, \
        HEAP_START, HEAP_SIZE, DEBUGGER, OS_BENCHMARKS, BENCHMARKS_ROOT_DIR, BUILD_DIR, BUILD_OPTIONS, \
        BENCH_TYPE, OS_RESULTS_DIR, BAREMETAL_RESULTS_DIR, BINS_DIR, BASELINE_DIR, BENCHMARK_VERIFICATION_DIR, \
        ANALYSIS_FILES_DIR, METRICS_RESULTS_DIR

    # open configuration file to setup global configuration values
    with open(IoT2_EVAL_CONFIG_FILE) as fd:
        IOT2_CONFIGS = json.load(fd)

        # get the path to the Benchiot/benchmarks directory
        BENCHMARKS_ROOT_DIR = PROJ_ROOT_DIR + IOT2_CONFIGS["BENCHMARKS_ROOT_DIR"]
        # get the name of the bins, verification, metrics, and analysis directories
        BINS_DIR = IOT2_CONFIGS[BINS_DIR]
        BENCHMARK_VERIFICATION_DIR = IOT2_CONFIGS[BENCHMARK_VERIFICATION_DIR]
        ANALYSIS_FILES_DIR = IOT2_CONFIGS[ANALYSIS_FILES_DIR]
        METRICS_RESULTS_DIR = IOT2_CONFIGS[METRICS_RESULTS_DIR]
        # get the name of the baseline directory
        BASELINE_DIR = IOT2_CONFIGS[BASELINE_DIR]
        # OS and baremetal dirs
        OS_RESULTS_DIR = IOT2_CONFIGS[OS_RESULTS_DIR]
        BAREMETAL_RESULTS_DIR = IOT2_CONFIGS[BAREMETAL_RESULTS_DIR]

        '''
        # check if we should choose baremetal or os benchmarks and set the path
        if IOT2_CONFIGS["OS_BENCHMARKS"] == 1:
            BENCHMARKS_PATH = PROJ_ROOT_DIR + IOT2_CONFIGS["OS_BENCHMARKS_DIR"]
            RESULTS_TYPE_DIR = IOT2_CONFIGS["OS_RESULTS_DIR"]
        else:
            BENCHMARKS_PATH = PROJ_ROOT_DIR + IOT2_CONFIGS["BAREMETAL_BENCHMARKS_DIR"]
            RESULTS_TYPE_DIR = IOT2_CONFIGS["BAREMETAL_RESULTS_DIR"]
        '''
        # setup DEBUGGER cmd
        DEBUGGER = IOT2_CONFIGS["DEBUGGER"]
        # setup how many times to repeat the experiments
        IOT2_ITERATIONS = IOT2_CONFIGS["ITERATIONS"]
        # setup board name
        BOARD_NAME = IOT2_CONFIGS["BOARD"]
        # setup the top result directory path
        RESULTS_DIR_PATH = PROJ_ROOT_DIR + IOT2_CONFIGS[RESULTS_DIR_PATH]
        #setup the build dir
        BUILD_DIR = IOT2_CONFIGS[BUILD_DIR]

        ''''# setup BINS_PATH (i.e., what bins to run) and METRICS_RESULTS path (i.e., where to write result files)
        if IOT2_CONFIGS["BASELINE_BENCHMARKS"] == 1:
            BINS_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["BASELINE"] + IOT2_CONFIGS["BINS_DIR"]
            METRICS_RESULTS_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["BASELINE"] + IOT2_CONFIGS["METRICS_RESULTS"]
            ANALYSIS_RESULTS_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["BASELINE"] + IOT2_CONFIGS["ANALYSIS_RESULTS"]
            BENCHMARKS_VERIFICATION_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["BASELINE"] + IOT2_CONFIGS["BENCHMARK_VERIFICATION"]
        else:
            BINS_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["MYTOOL"] + IOT2_CONFIGS["BINS_DIR"]
            METRICS_RESULTS_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["MYTOOL"] + IOT2_CONFIGS["METRICS_RESULTS"]
            ANALYSIS_RESULTS_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["MYTOOL"] + IOT2_CONFIGS["ANALYSIS_RESULTS"]
            BENCHMARKS_VERIFICATION_PATH = RESULTS_DIR_PATH + IOT2_CONFIGS["MYTOOL"] +\
                                           IOT2_CONFIGS["BENCHMARK_VERIFICATION"]
        '''

        # setup breakpoint function for benchmarks, this is used by the debugger script to automate running
        # and stopping the benchmarks
        BENCHMARK_END = IOT2_CONFIGS["BENCHMARK_END_BREAKPOINT"]

        # setup serial port to use for benchmarks and for collecting data
        IOT2_SERIAL_PORT = IOT2_CONFIGS["IOT2_SERIAL_PORT"]

        # setup network configurations
        BOARD_IP = IOT2_CONFIGS["BOARD_IP"]
        BOARD_PORT = IOT2_CONFIGS["BOARD_PORT"]
        PC_IP = IOT2_CONFIGS["PC_IP"]

        # setup the bootloader file
        BOOTLOADER_BIN_FILE = IOT2_CONFIGS["BOOTLOADER_BIN_FILE"]

        # setup the BOARD_BUG_BKPT option
        BOARD_BUG_BKPT = IOT2_CONFIGS["BOARD_BUG_BKPT"]

        # setup metrics organization
        METRIC_CATAGORIES = IOT2_CONFIGS["METRIC_CATAGORIES"]
        SECURITY_METRICS = IOT2_CONFIGS["METRIC_CATAGORIES"]["SECURITY_METRICS"]
        PERFORMANCE_METRICS = IOT2_CONFIGS["METRIC_CATAGORIES"]["PERFORMANCE_METRICS"]
        FLASH_METRICS = IOT2_CONFIGS["METRIC_CATAGORIES"]["FLASH_METRICS"]
        RAM_METRICS = IOT2_CONFIGS["METRIC_CATAGORIES"]["RAM_METRICS"]
        ENERGY_METRICS = IOT2_CONFIGS["METRIC_CATAGORIES"]["ENERGY_METRICS"]

        # setup the manual benchmarks list
        MANUAL_BENCH_LIST = IOT2_CONFIGS["MANUAL_BENCH_LIST"]

        # setup dynamic memory configuration
        STACK_TOP = int(IOT2_CONFIGS["DYNAMIC_MEMRORY"]["STACK_TOP"], 16)
        STACK_SIZE = IOT2_CONFIGS["DYNAMIC_MEMRORY"]["STACK_SIZE"]
        HEAP_START = int(IOT2_CONFIGS["DYNAMIC_MEMRORY"]["HEAP_START"], 16)
        HEAP_SIZE = IOT2_CONFIGS["DYNAMIC_MEMRORY"]["HEAP_SIZE"]

        # setup type of benchmarks (bare-metal/OS)
        OS_BENCHMARKS = IOT2_CONFIGS["OS_BENCHMARKS"]

        # build options
        BUILD_OPTIONS = {
            # ----------------------------------------------------------------------------------
            # OS
            # ----------------------------------------------------------------------------------
            'mbed-os': {
                MBED_CMD: ['mbed', 'export', '-m', str(BOARD_NAME), '-i', 'GCC_ARM', '--profile',
                           'release'],
                CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1',
                            'CUSTOM_BIN_SUFFIX=--baseline',
                            'clean'],
                MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1',
                           'CUSTOM_BIN_SUFFIX=--baseline',
                           'iot2benchmark'],
                BENCH_TYPE : str(OS_RESULTS_DIR),
                BENCH_CONFIG_RES_DIR: str(BASELINE_DIR)
            },
            'uvisor': {
                MBED_CMD: ['mbed', 'export', '-m', str(BOARD_NAME), '-i', 'GCC_ARM', '--profile',
                           'release'],
                CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_UVISOR=1',
                            'IOT2_CONFIG=3', 'CUSTOM_LINKER=uvisor', 'CUSTOM_BIN_SUFFIX=--uvisor', 'clean'],
                MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_UVISOR=1',
                           'IOT2_CONFIG=3', 'CUSTOM_LINKER=uvisor', 'CUSTOM_BIN_SUFFIX=--uvisor', 'iot2benchmark'],
                BENCH_TYPE : str(OS_RESULTS_DIR),
                BENCH_CONFIG_RES_DIR: '/uvisor'
            },
            'secure_data_OS': {
                MBED_CMD: ['mbed', 'export', '-m', str(BOARD_NAME), '-i', 'GCC_ARM', '--profile',
                           'release'],
                CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_CONFIG=2',
                            'CUSTOM_LINKER=securedata', 'CUSTOM_BIN_SUFFIX=--secure_data_OS', 'clean'],
                MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_CONFIG=2',
                           'CUSTOM_LINKER=securedata', 'CUSTOM_BIN_SUFFIX=--secure_data_OS', 'iot2benchmark'],
                BENCH_TYPE : str(OS_RESULTS_DIR),
                BENCH_CONFIG_RES_DIR: '/secure_data_OS'
            },
            'secure_watchdog': {
                MBED_CMD: ['mbed', 'export', '-m', str(BOARD_NAME), '-i', 'GCC_ARM', '--profile',
                           'release'],
                CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_CONFIG=1',
                            'CUSTOM_LINKER=watchdog', 'CUSTOM_BIN_SUFFIX=--secure_watchdog', 'clean'],
                MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_CONFIG=1',
                           'CUSTOM_LINKER=watchdog', 'CUSTOM_BIN_SUFFIX=--secure_watchdog', 'iot2benchmark'],
                BENCH_TYPE: str(OS_RESULTS_DIR),
                BENCH_CONFIG_RES_DIR: '/secure_watchdog'
            },

            # ----------------------------------------------------------------------------------
            # Bare-metal
            # ----------------------------------------------------------------------------------
            'mbed': {
                MBED_CMD: ['mbed', 'export', '-m', str(BOARD_NAME), '-i', 'GCC_ARM', '--profile',
                           'release'],
                CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_BM=1',
                            'CUSTOM_BIN_SUFFIX=--baseline',
                            'clean'],
                MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_BM=1',
                           'CUSTOM_BIN_SUFFIX=--baseline',
                           'iot2benchmark'],
                BENCH_TYPE: str(BAREMETAL_RESULTS_DIR),
                BENCH_CONFIG_RES_DIR: str(BASELINE_DIR)
            },
            'secure_data_SDK': {
                MBED_CMD: ['mbed', 'export', '-m', str(BOARD_NAME), '-i', 'GCC_ARM', '--profile',
                           'release'],
                CLEAN_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_BM=1', 'IOT2_CONFIG=2',
                            'CUSTOM_LINKER=securedata', 'CUSTOM_BIN_SUFFIX=--secure_data_SDK', 'clean'],
                MAKE_CMD: ['make', '-j4', 'BOARD=' + str(BOARD_NAME), 'IOT2_ALL=1', 'IOT2_BM=1', 'IOT2_CONFIG=2',
                           'CUSTOM_LINKER=securedata', 'CUSTOM_BIN_SUFFIX=--secure_data_SDK', 'iot2benchmark'],
                BENCH_TYPE: str(BAREMETAL_RESULTS_DIR),
                BENCH_CONFIG_RES_DIR: '/secure_data_SDK'
            },
            # ----------------------------------------------------------------------------------
        }
    return

# initialize configurations automatically
init_configs()
