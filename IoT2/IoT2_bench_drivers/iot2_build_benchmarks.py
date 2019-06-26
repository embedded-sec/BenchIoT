"""
This file is used to automate building and copying the benchmarks binaries to
the corresponding directory under the results directory
"""

import os
import sys
import argparse
import numpy
import json
import pprint
import subprocess
import errno

import iot2_settings, iot2_run_experiments
import iot2_measure_static_flash_and_ram as iot2_helper
#######################################################################
#                              GLOBALS                                #
#######################################################################

# all globals are setup in iot2_settings

#######################################################################
#                              CLASSES                                #
#######################################################################



#######################################################################
#                              FUNCTIONS                              #
#######################################################################




def gen_benchmarks_dir_list(dirs_path, suffix=None):
    """
    Returns a list of directories for the given path
    :param dirs_path: Path to ti check the list of of directories
    :return: list of directories for the given path
    """
    dirs_list = []
    final_dir_list = []
    for dirname in os.listdir(dirs_path):
        if os.path.isdir(os.path.join(dirs_path, dirname)):
            dirs_list.append(str(dirname))

    if suffix:
        for dir_name in dirs_list:
            if suffix in str(dir_name):
                final_dir_list.append(dir_name)
    else:
        final_dir_list = dirs_list

    return final_dir_list


def build_bencmark(build_option=iot2_settings.ALL_STR, benchmark_name=iot2_settings.ALL_STR, disable_clean=False, disable_export=False):
    benchmark_substr = '-benchmarks'
    bench_configs_substr = benchmark_substr
    bench_name_substr = ""

    # are we bulding for only one configuratios (e.g., mbed-os) or for all configs in BUILD_OPTIONS
    if build_option != iot2_settings.ALL_STR:
        bench_configs_substr = build_option + benchmark_substr
    # are we building all benchmarks or a specific benchmark
    if benchmark_name != iot2_settings.ALL_STR:
        bench_name_substr = benchmark_name

    # get configuratios list by looking at dirctories under Benchiot/benchmarks
    bench_configs = gen_benchmarks_dir_list(iot2_settings.BENCHMARKS_ROOT_DIR, bench_configs_substr)

    # build each configuratio one by one
    for bench_config in bench_configs:
        bench_config_path = os.path.join(iot2_settings.BENCHMARKS_ROOT_DIR, bench_config)#iot2_settings.BENCHMARKS_ROOT_DIR + bench_config
        bench_config_name = bench_config.replace(benchmark_substr, "")
        print("="*80)
        print("[*] Benchmark configuraiton: " +bench_config_path)
        benchmarks_list = gen_benchmarks_dir_list(bench_config_path, bench_name_substr)
        for benchmark_dir in benchmarks_list:
            benchmark_path = os.path.join(bench_config_path, benchmark_dir)
            print("\t"+"-"*80)
            print("\t [BENCMARK]: %s" % benchmark_path)
            os.chdir(benchmark_path)
            with open("auto_build.log", 'wt') as log_fd:
                if not disable_export:
                    print("\t [+] Creating makefile")
                    ret = subprocess.call(iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.MBED_CMD], stdout=log_fd, stderr=subprocess.STDOUT)
                    # call mbed-cli to create makefile
                    if ret != 0:
                        print("\t [ERROR] Failed to create makefile for: %s" % str(benchmark_path))
                        # no point of continuing since there is not makefile, move ot the next benchmark
                        continue

                if not disable_clean:
                    # do a make clean before building the benchmark
                    ret = subprocess.call(iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.CLEAN_CMD], stdout=log_fd,
                                          stderr=subprocess.STDOUT)
                    if ret != 0:
                        print("\t [ERROR] Failed to do make clean: %s" % str(benchmark_path))
                        continue
                # build the benchmark
                print("\t [MAKE] %s" % str(benchmark_path))
                ret = subprocess.call(iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.MAKE_CMD], stdout=log_fd,
                                      stderr=subprocess.STDOUT)
                if ret != 0:
                    print("\t [ERROR] Failed to build: %s" % str(benchmark_path))
                    continue
                else:
                    print("\t [+] Build succeeded")

                # copy the benchmark binaries to the results binary to automate running the benchmark
                bin_files_list = []
                bin_files_list = iot2_helper.gen_filelist(str(os.path.join(benchmark_path + iot2_settings.BUILD_DIR)),
                                                          bin_files_list, iot2_settings.BIN_EXT)

                ret = cp_bin_to_res_dir(bin_files_list, bench_config_name, log_fd)
                if ret != 0:
                    print("\t [ERROR] Failed to copy elf file to results directory:")
                    continue
                else:
                    print("\t [+] Benchmark <.elf> file copied to %s/bins in results directory" % bench_config_name)
            print("\t"+"-" * 80)
        print("=" * 80)


def cp_bin_to_res_dir(bin_files, dist_dir_name, log_fd):
    for bin_file in bin_files:
        res_path = str(iot2_settings.RESULTS_DIR_PATH +
                       iot2_settings.BUILD_OPTIONS[dist_dir_name][iot2_settings.BENCH_TYPE] +
                       iot2_settings.BUILD_OPTIONS[dist_dir_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                       iot2_settings.BINS_DIR)
        cpy_cmd = ['cp', bin_file, res_path]
        ret = subprocess.call(cpy_cmd, stdout=log_fd, stderr=subprocess.STDOUT)
        return ret


#######################################################################
#                                 MAIN                                #
#######################################################################

if __name__ == '__main__':
    try:
        parser = argparse.ArgumentParser()
        parser.add_argument('-bc', '--benchmark-config', dest='benchmark_configuration', type=str,
                            help="Flag to set the benchmark configuration to use from iot2_settings.BUILD_OPTIONS. if"
                                 "not set, then all configurations are built."
                            , default=iot2_settings.ALL_STR)
        parser.add_argument('-bn', '--benchmark-name', dest='benchmark_name', type=str,
                            help="Flag to set the benchmark to build. Default value is to build all benchmarks",
                            default=iot2_settings.ALL_STR)
        parser.add_argument('-dc', '--disable-clean', dest='disable_clean', default=False,
                            action='store_true', help="Using this options disables running [make ... clean].")
        parser.add_argument('-de', '--disable-export', dest='disable_export', default=False,
                            action='store_true', help="Using this options disables running [mbed export ... ] to "
                                                      "re-write the makefile.")

        args = parser.parse_args()

        build_bencmark(args.benchmark_configuration, args.benchmark_name, args.disable_clean, args.disable_export)
    except Exception as e:
        raise
