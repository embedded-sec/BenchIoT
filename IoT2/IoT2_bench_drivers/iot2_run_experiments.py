import os
import importlib
import signal
import time
import socket
import sys
import argparse
import numpy
import shutil
import threading
from threading import Event
import multiprocessing
from multiprocessing import Lock, Value
# IoT2 imports
import iot2_settings
import setup_result_collection
import iot2_measure_static_flash_and_ram as iot2_helper
import iot2_result_collector
import saleae
# import benchmarks drivers
#import iot2_tcp_drivers

#######################################################################
#                   GLOBAL VARIABLS FOR EXPERIMENTS                   #
#######################################################################
LOGIC_HOST = 'localhost'
LOGIC_PORT = 10429
DEFAULT_OUTPUT_VOLTAGE = 3.0
DELAY_TO_AVOID_RACE_CONDITION = 2
DIGITAL_CHANNELS = [1]
ANALOG_CHANNELS = [0,1]
TRIGGER_CHANNEL = 2
TRIGGER_TYPE = saleae.Trigger.Posedge
RECORD_LENGTH = 30*60
#######################################################################
#                              FUNCTIONS                              #
#######################################################################
def init_saleae():
    '''
        Connects to SALEAE Logic, loads the config file and returns
        the connection
    '''
    saleae_con= saleae.Saleae(LOGIC_HOST,LOGIC_PORT)

    # config_saleae(saleae_con)
    return saleae_con

# def config_saleae(saleae_con):
#     saleae_con.set_active_channels(DIGITAL_CHANNELS, ANALOG_CHANNELS)
#     saleae_con.set_trigger_one_channel(TRIGGER_CHANNEL, TRIGGER_TYPE)

def gen_dir_list(dirs_path, benchmark=None):
    """
    Returns a list of directories for the given path
    :param dirs_path: Path to ti check the list of of directories
    :return: list of directories for the given path
    """
    dirs_list = []
    for dirname in os.listdir(dirs_path):
        if os.path.isdir(os.path.join(dirs_path, dirname)):
            dirs_list.append(dirname)
    if benchmark:
        if benchmark == iot2_settings.ALL:
            return dirs_list
        elif benchmark in dirs_list:
            dirs_list = []
            dirs_list.append(benchmark)
        else:
            print("[-] No benchmark has the name <%s>. EXITING....." % benchmark)
            sys.exit(0)
    return dirs_list


def copy_bins(benchmarks_path, benchmark_name=None):
    bench_dir_list = gen_dir_list(benchmarks_path, benchmark_name)
    # setup the full path for each benchmark binary directory
    bench_bins_list_with_full_path = []
    for bench_name in bench_dir_list:
        # setup path to the binaries according to config file
        curr_bench_path = str(os.path.join(benchmarks_path, bench_name)) +\
                          str(iot2_settings.BUILD_DIR) + str(iot2_settings.BOARD_NAME)
        current_bench_bin = []
        current_bench_bin = iot2_helper.gen_filelist(curr_bench_path,
                                                     current_bench_bin,
                                                     iot2_settings.BIN_EXT)
        for bin_file in current_bench_bin:
            # append binary files for current benchmark
            bench_bins_list_with_full_path.append(bin_file)
    # copy binaries to bins folder in the results
    for bin_file in bench_bins_list_with_full_path:
        print("[+] Copying <%s> to %s" % (bin_file, iot2_settings.BINS_PATH))
        shutil.copy(bin_file, iot2_settings.BINS_PATH)
    print("[+] Done.....")


def get_benchmark_bin(bench_config_name, benchmark_name):
    # get bench_config bins path
    bench_config_path = str(iot2_settings.RESULTS_DIR_PATH +
                            iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] +
                            iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                            iot2_settings.BINS_DIR)
    bin_list = []
    bin_list = iot2_helper.gen_filelist(str(bench_config_path), bin_list, iot2_settings.BIN_EXT)
    for bin_file in bin_list:
        if benchmark_name in bin_file:
            return bin_file
    print("[-] ERROR: could not find binary for <%s> benchmark" % benchmark_name)
    return "NO-FILE-FOUND"


# process to run gdb with the app
def run_debug(bin_file, silent_flag):
    cmd_str = iot2_settings.DEBUGGER + ' ' + bin_file + ' -x ' + iot2_settings.GDB_SCRIPT + ' ' + silent_flag
    os.system(cmd_str)


# returns the app_name from the full path
def get_app_name(iot_app_path):
    res = str(iot_app_path).rsplit("/")
    return str(str(res[len(res) - 1]).split('.')[0])


def get_net_driver_func(benchmark_name):
    """
    :param benchmark_name: This MUST be the exact name of the actual benchmark (tcp_driver_name === benchmark_name)
    :return: The net_driver function of the given benchmark
    """
    tcp_driver_name = "." + benchmark_name
    net_driver_func = importlib.import_module(tcp_driver_name, package='iot2_tcp_drivers')
    return net_driver_func


def run_benchmark(bench_config_name, benchmark_name, append_results, logic_analyzer):
    bin_file = get_benchmark_bin(bench_config_name, benchmark_name)
    print("-" * 80)
    print(bin_file)
    print("-" * 80)
    # process for running the benchmark (i.e., with the debugger)
    benchmark_proc = threading.Thread(target=run_debug, args=(bin_file, silent_flag))
    verification_path = str(iot2_settings.RESULTS_DIR_PATH +
                            iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] +
                            iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                            iot2_settings.BENCHMARK_VERIFICATION_DIR)

    verification_file = verification_path + benchmark_name + iot2_settings.TXT_EXT


    # check if benchmark is an OS/baremetal benchmark.
    if iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] == iot2_settings.OS_RESULTS_DIR:
        os_benchmark_flag = 1
    else:
        os_benchmark_flag = 0
    try:

        if logic_analyzer is not None:
            logic_analyzer.set_capture_seconds(RECORD_LENGTH)
            
            logic_analyzer.capture_start()

        # process to run the results collector
        result_collection_proc = multiprocessing.Process(
            target=iot2_result_collector.iot2_serial_collector,
            args=(iot2_settings.IOT2_SERIAL_PORT, iot2_settings.IOT2_ITERATIONS,
                  bench_config_name, benchmark_name, append_results, iot2_synch_obj))
        # process for the network driver
        # get the net_driver function for the given benchmark
        net_driver_func = get_net_driver_func(benchmark_name)
        # setup the tcp_driver process
        tcp_driver_proc = multiprocessing.Process(target=net_driver_func.net_driver,
                                                  args=(iot2_settings.BOARD_IP, iot2_settings.BOARD_PORT,
                                                        iot2_settings.IOT2_ITERATIONS,
                                                        verification_file,
                                                        iot2_settings.BOOTLOADER_BIN_FILE,
                                                        iot2_settings.BOARD_NAME,
                                                        os_benchmark_flag,
                                                        iot2_synch_obj))
        # set processes as daemons
        result_collection_proc.daemon = True
        benchmark_proc.daemon = True
        tcp_driver_proc.daemon = True
        # start processes
        tcp_driver_proc.start()
        result_collection_proc.start()
        benchmark_proc.start()

        # wait for processes to end
        while result_collection_proc.is_alive() or benchmark_proc.is_alive():
            time.sleep(2)
        # end processes
        tcp_driver_proc.join()
        result_collection_proc.join()
        benchmark_proc.join()
        if logic_analyzer is not None:
            raw_energy_file = str(iot2_settings.RESULTS_DIR_PATH +
                iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] +
                iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                iot2_settings.METRICS_RESULTS_DIR) + benchmark_name + '_energy'
            print "Saving Energy to: ", raw_energy_file
            logic_analyzer.capture_start() # May have to manually do this
            logic_analyzer.save_to_file(raw_energy_file+".logicdata")
            logic_analyzer.export_data2(raw_energy_file+".csv")
    except KeyboardInterrupt:
        if tcp_driver_proc.is_alive():
            tcp_driver_proc.terminate()
        if result_collection_proc.is_alive():
            result_collection_proc.terminate()
        print("Exception...")
        sys.exit()
    return


#######################################################################
#                                CLASSES                              #
#######################################################################

class IOT2SynchronizationObject(object):
    def __init__(self):
        self.tcp_proc_started = Value('i', 0)
        self.benchmark_status = Value('i', 0)
        self.lock = Lock()

    def set_benchmark_status(self):
        with self.lock:
            self.benchmark_status.value = 1

    def get_benchmark_status(self):
        with self.lock:
            return self.benchmark_status.value

    def reset_benchmark_status(self):
        with self.lock:
            self.benchmark_status.value = 0

    def set_tcp_proc_flag(self):
        with self.lock:
            self.tcp_proc_started.value = 1

    def get_tcp_proc_flag(self):
        with self.lock:
            return self.tcp_proc_started.value

    def reset_tcp_proc_flag(self):
        with self.lock:
            self.tcp_proc_started.value = 0



#######################################################################
#                                 MAIN                                #
#######################################################################
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--all', dest='run_all', default=False,
                        action='store_true',
                        help="Flag to indicate if all measurement (both benchmarks and IoT) should be measured")
    parser.add_argument('-bc', '--benchmark-config', dest='benchmark_configuration', type=str,
                        help="Flag to set the benchmark configuration to use from iot2_settings.BUILD_OPTIONS. if"
                             "not set, then all configurations are built."
                        , default=iot2_settings.ALL_STR)
    parser.add_argument('-bn', '--benchmark-name', dest='benchmark_name', type=str,
                        help="Flag to set the benchmark to build. Default value is to build all benchmarks",
                        default=iot2_settings.ALL_STR)
    parser.add_argument('-l', '--list', dest='list', default=False,
                        action='store_true', help="Use the MANUAL_BENCH_LIST form evaluation-results-config.json")
    parser.add_argument('-s', '--run-silently', dest='run_silent', default=False,
                        action='store_true', help="Flag to run benchmark silently (i.e., with stdout)")
    parser.add_argument('-ap', '--append-results', dest='append_results', default=False,
                        action='store_true', help="Flag to append results to a the file from a previous run")
    parser.add_argument('-test', '--testing', dest='test_settings', default=False,
                        action='store_true', help="Flag to use for quick testing and debugging.")
    parser.add_argument('-e', '--energy', dest='energy', default=False,
                        action='store_true', help="Flag to use for capturing energy using saleae Logic.")

    args = parser.parse_args()
    try:
        # initialize synchoronization object
        iot2_synch_obj = IOT2SynchronizationObject()

        print("*"*80)
        print("START")
        print("*"*80)

        if args.energy:
            logic_analyzer = init_saleae()
        else:
            logic_analyzer = None

        if args.run_silent:
            silent_flag = '-q -batch-silent'
        else:
            silent_flag = ''

        # if running all benchmarks
        if args.run_all:
            print("[-] running all is not available at the moment")

        elif args.test_settings:
            test_bin_file = get_benchmark_bin("smart_locker")
            print(test_bin_file)

        # if running only the manual list of benchmarks from the json configuration file
        elif args.list:
            for bench in iot2_settings.MANUAL_BENCH_LIST:
                print("-" * 80)
                print("[+] RUNNING BENCHMARK: %s" % bench)
                run_benchmark(args.benchmark_configuration, bench, args.append_results, logic_analyzer)
                print("-" * 80)

        # if running only one benchmarks
        elif args.benchmark_name:
            print("benchmark name = %s" % args.benchmark_name)
            run_benchmark(args.benchmark_configuration, args.benchmark_name, args.append_results, logic_analyzer)

        else:
            print("[-] Error: Either --all, or -b should be used.")
            parser.print_usage()

    except KeyboardInterrupt:
        print("[-] CTRL-C pressed. Exititng....")

    except:
        raise
