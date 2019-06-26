import os
import sys
import argparse
import numpy
import json
import pprint
import subprocess
import errno

import iot2_settings, iot2_run_experiments
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


def create_res_dirs(json_config_file, results_path):
    """
    Creates the list of directories given in the configuration file. This is used to create the directories will be
    using for results.
    :param json_config_file: The configuration file to specify the result directory structure.
    :return: None
    """
    # these are variables representing the strings we had setup in the json configuration file
    json_results_dirname = "RESULTS_DIR"
    json_results_subdir_strct_name = "RESULTS_SUBDIRS_STRUCTURE"
    json_subdir_strct_name = "SUBDIRS"
    json_dirs_name = "DIRS"
    json_none_name = "NONE"

    # read results-configuration json file and create the directories
    results_config = json.load(open(json_config_file))
    results_subdir_strct = results_config[json_results_subdir_strct_name]
    subdir_strct = results_subdir_strct[json_subdir_strct_name]
    dirs_list = results_subdir_strct[json_dirs_name]

    # make directories recursivley as specified in json file
    dir_path = results_path + results_config[json_results_dirname]
    print("=" * 80)
    mkdir_recursive(subdir_strct, dirs_list, dir_path, json_none_name)
    print("=" * 80)
    return


def mkdir_recursive(subs, dirs, dir_path, none_id):
    if subs == none_id:
        print("-"*80)
        for dir_name in dirs:
            try:
                final_dir_path = str(dir_path+dir_name)
                print("[+] Creating directory: " + final_dir_path)
                os.makedirs(final_dir_path)
            except OSError as e:
                if e.errno == errno.EEXIST and os.path.isdir(final_dir_path):
                    pass
                else:
                    raise
        return
    else:
        for dir_name in dirs:
            mkdir_recursive(subs["SUBDIRS"], subs["DIRS"], dir_path+dir_name, none_id)



def create_fixed_results_dir(json_config_file, results_path):
    # read results-configuration json file and create the directories
    with open(json_config_file) as fd:
        results_config = json.load(fd)
        results_dir_json_config = "RESULTS_DIR"
        baseline_json_config = "BASELINE_BENCH_DIR"
        bench_types_json_config = "BENCH_RESULT_TYPES"
        bm_bench_tools_json_config = "BAREMETAL_BENCH_TOOLS"
        os_bench_tools_json_config = "OS_BENCH_TOOLS"
        bench_subdirs_json_config = "BENCH_DIR_SUBDIRS"
        bm_res_json_config = "BAREMETAL_RESULTS_DIR"
        os_res_json_config = "OS_RESULTS_DIR"

        results_root_dir = results_path + results_config[results_dir_json_config]
        baseline_dir = results_config[baseline_json_config]
        bench_types_dirs = results_config[bench_types_json_config]
        bm_bench_tools_dirs = results_config[bm_bench_tools_json_config]
        os_bench_tools_dirs = results_config[os_bench_tools_json_config]
        bench_subdirs = results_config[bench_subdirs_json_config]
        bm_res_dir = str(results_root_dir + results_config[bm_res_json_config])
        os_res_dir = str(results_root_dir + results_config[os_res_json_config])

        # create baremetal results dirs
        mksubdirs_fixed_results(os_res_dir, os_bench_tools_dirs, baseline_dir, bench_subdirs)
        mksubdirs_fixed_results(bm_res_dir, bm_bench_tools_dirs, baseline_dir, bench_subdirs)
        # create os results dirs
    return



def mksubdirs_fixed_results(bench_type_dir, tools_list_dirs, baseline_dir, bench_subdirs_list):
    # creat baseline
    baseline_dir_path = bench_type_dir + baseline_dir
    for bench_subdir in bench_subdirs_list:
        final_dir_path = baseline_dir_path + bench_subdir
        try:
            print("[+] Creating directory: " + final_dir_path)
            os.makedirs(final_dir_path)
        except OSError as e:
            if e.errno == errno.EEXIST and os.path.isdir(final_dir_path):
                pass
            else:
                raise

    for tool_name in tools_list_dirs:
        for bench_subdir in bench_subdirs_list:
            final_dir_path = bench_type_dir + tool_name + bench_subdir
            try:
                print("[+] Creating directory: " + final_dir_path)
                os.makedirs(final_dir_path)
            except OSError as e:
                if e.errno == errno.EEXIST and os.path.isdir(final_dir_path):
                    pass
                else:
                    raise
    return

#######################################################################
#                                 MAIN                                #
#######################################################################

if __name__ == '__main__':
    try:
        parser = argparse.ArgumentParser()
        parser.add_argument('-p', '--path', dest='IoT2_RESULTS_PATH', type=str,
                            help="Flag to set the output path of the results, if not set the results directory in "
                                 "the project root is used.", default=iot2_settings.PROJ_ROOT_DIR)
        args = parser.parse_args()
        #print(args.IoT2_RESULTS_PATH)
        #create_res_dirs(iot2_settings.IoT2_EVAL_CONFIG_FILE, args.IoT2_RESULTS_PATH)
        create_fixed_results_dir(iot2_settings.IoT2_EVAL_CONFIG_FILE, args.IoT2_RESULTS_PATH)

    except Exception as e:
        raise
