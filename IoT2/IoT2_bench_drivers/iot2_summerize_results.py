import os
import sys
import argparse
import numpy as np
import json
import pprint
import time
from datetime import datetime
from collections import OrderedDict
import matplotlib.pyplot as plt
import numpy as np

# IoT2 imports
import iot2_settings
import setup_result_collection
import iot2_measure_static_flash_and_ram as iot2helper
import iot2_run_experiments as iot2_exp
import subprocess


#######################################################################
#                              GLOBALS                                #
#######################################################################

FIG_SIZE = (3.5, 2.5)


#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def calc_runtime_cycles(runtime_arr, data_size):
    # the size of the data will be equal to the iterations unless stated otherwise
    total_runtime_cycles_arr = []
    num_overflows_opt = iot2_settings.RUNTIME_NUM_OVERFLOWS
    total_runtime_opt = iot2_settings.RUNTIME_CYCLES
    max_cycles = iot2_settings.CYCLE_COUNTER_MAX
    for i in range(data_size):
        runtime_cycles = (runtime_arr[num_overflows_opt][i] * max_cycles) + runtime_arr[total_runtime_opt][i]
        total_runtime_cycles_arr.append(runtime_cycles)
    return total_runtime_cycles_arr


def get_csv_result_file(benchmark_name, results_path):
    json_res_file = results_path + benchmark_name + iot2_settings.JSON_EXT
    csv_res_file = results_path + benchmark_name + iot2_settings.CSV_EXT
    json_res = json.load(json_res_file)
    #with open(csv_res_file, 'w') as csv_fd:


def plot_benchmark(benchmark_name, results_path):
    res_file_path = results_path + benchmark_name + iot2_settings.JSON_EXT
    with open(res_file_path) as fd:
        results = json.load(fd)
    runtime_arr = results[iot2_settings.TOTAL_RUNTIME_DICTIONARY]
    total_runtime_arr = calc_runtime_cycles(runtime_arr, iot2_settings.IOT2_ITERATIONS)
    print(runtime_arr)
    print("-"*80)
    print(total_runtime_arr)
    print("-"*80)
    print(np.round(np.mean(total_runtime_arr)))
    return


def get_metric_catagory(metric_name):
    catagory_name = ""

    for metric_key in iot2_settings.METRIC_CATAGORIES.keys():
        if metric_name in iot2_settings.METRIC_CATAGORIES[metric_key]:
            return str(metric_key)
    return "OTHER_METRIC"


def gen_all_bench_result_file(bench_config_name):
    metrics_results_path = str(iot2_settings.RESULTS_DIR_PATH +
                               iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] +
                               iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                               iot2_settings.METRICS_RESULTS_DIR)

    all_benches_result_file = metrics_results_path + iot2_settings.ALL_BENCHES_RESULT_FILE + \
                              iot2_settings.JSON_EXT

    final_results_file = metrics_results_path + "FINAL_RESULTS" + iot2_settings.JSON_EXT
    final_stddev = []
    file_list = []
    file_list = iot2helper.gen_filelist(metrics_results_path, file_list, iot2_settings.JSON_EXT)
    # remove the all bench results file if it already exists
    if all_benches_result_file in file_list:
        file_list.remove(all_benches_result_file)
    if final_results_file in file_list:
        file_list.remove(final_results_file)
    # creat dict for all results
    all_results_dict = OrderedDict()
    for res_file in file_list:
        with open(res_file, 'r') as fd:
            # creata a dictionary for each type
            security_dict = OrderedDict()
            perf_dict = OrderedDict()
            flash_dict = OrderedDict()
            ram_dict = OrderedDict()
            energy_dict = OrderedDict()
            other_dict = OrderedDict()
            # read benchmarks json file
            bench_results = json.load(fd)
            # catagorize each metric according to the json configuration file
            for metric in bench_results:
                if 'Max_code_ratio' in metric or 'Max_data_ratio' in metric:
                    metric_val = np.mean(bench_results[metric])
                else:
                    metric_val = int(np.mean(bench_results[metric]))
                if metric == 'TotalRuntime_cycles':
                    std_val = (np.std(bench_results[metric], ddof=1)/metric_val)*100

                category_name = get_metric_catagory(metric)
                # add metric to its category dictionary
                if category_name == iot2_settings.SECURITY_METRICS_TYPE:
                    security_dict[metric] = metric_val
                elif category_name == iot2_settings.PERFORMANCE_METRICS_TYPE:
                    perf_dict[metric] = metric_val
                elif category_name == iot2_settings.FLASH_METRICS_TYPE:
                    flash_dict[metric] = metric_val
                elif category_name == iot2_settings.RAM_METRICS_TYPE:
                    ram_dict[metric] = metric_val
                elif category_name == iot2_settings.ENERGY_METRICS_TYPE:
                    energy_dict[metric] = metric_val
                else:
                    other_dict[metric] = metric_val
        # create a dict for the current benchmark, each benchmark will have its results catagorized
        # in the json file
        bench_dict = OrderedDict()
        bench_dict[iot2_settings.SECURITY_METRICS_TYPE] = security_dict
        bench_dict[iot2_settings.PERFORMANCE_METRICS_TYPE] = perf_dict
        bench_dict[iot2_settings.FLASH_METRICS_TYPE] = flash_dict
        bench_dict[iot2_settings.RAM_METRICS_TYPE] = ram_dict
        bench_dict["OTHER_METRICS"] = other_dict        # this dictionary is for addtional/analysis metrics
        bench_name = iot2_exp.get_app_name(res_file)
        # add the benchmark to the all benchmarks dictionary
        all_results_dict[bench_name] = bench_dict

        # append stddev
        final_stddev.append(std_val)
    # write the final results file
    with open(all_benches_result_file, 'w') as final_fd:
        json.dump(all_results_dict, final_fd, sort_keys=True, indent=4, ensure_ascii=False)

    print("-" * 80)
    print(final_stddev)
    print("-" * 80)


def gen_final_results(bench_config_name):

    metrics_results_path = str(iot2_settings.RESULTS_DIR_PATH +
                               iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] +
                               iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                               iot2_settings.METRICS_RESULTS_DIR)
    all_benches_result_file = metrics_results_path + iot2_settings.ALL_BENCHES_RESULT_FILE + \
                              iot2_settings.JSON_EXT

    final_results_file = metrics_results_path + "FINAL_RESULTS" + iot2_settings.JSON_EXT

    if 'os_results' in metrics_results_path:
        baseline_file = str(iot2_settings.RESULTS_DIR_PATH +
                            iot2_settings.BUILD_OPTIONS['mbed-os'][iot2_settings.BENCH_TYPE] +
                            iot2_settings.BUILD_OPTIONS['mbed-os'][iot2_settings.BENCH_CONFIG_RES_DIR] +
                            iot2_settings.METRICS_RESULTS_DIR) + iot2_settings.ALL_BENCHES_RESULT_FILE + iot2_settings.JSON_EXT
    else:
        baseline_file = str(iot2_settings.RESULTS_DIR_PATH +
                            iot2_settings.BUILD_OPTIONS['mbed'][iot2_settings.BENCH_TYPE] +
                            iot2_settings.BUILD_OPTIONS['mbed'][iot2_settings.BENCH_CONFIG_RES_DIR] +
                            iot2_settings.METRICS_RESULTS_DIR) + iot2_settings.ALL_BENCHES_RESULT_FILE +iot2_settings.JSON_EXT
    # setup paths
    tool_file = all_benches_result_file
    final_res_file = final_results_file

    with open(baseline_file, 'r') as b_fd:
        baseline_res = json.load(b_fd)
    with open(tool_file, 'r') as t_fd:
        tool_res = json.load(t_fd)

    # [iot2-debug]: remove PrivCode from measurement to avoid / 0
    security_metrics = iot2_settings.SECURITY_METRICS
    security_metrics.remove("TotalPriv_code")

    res_dict = OrderedDict()
    for bench_name in baseline_res:
        bench_dict = OrderedDict()
        tool_runtime = tool_res[bench_name]['PERFORMANCE_METRICS']['TotalRuntime_cycles']
        for catagory in baseline_res[bench_name]:
            catagory_dict = OrderedDict()
            for metric in baseline_res[bench_name][catagory]:
                print("-"*80)
                print(metric)
                print("-"*80)
                baseline_metric = baseline_res[bench_name][catagory][metric]
                tool_metric = tool_res[bench_name][catagory][metric]
                proportion = ""
                if 'cycles' in metric and 'TotalRuntime' not in metric:
                    #print("$"* 80)
                    #print(metric)
                    #print(tool_metric)
                    #print(tool_runtime)
                    #print(float(tool_metric)/tool_runtime)
                    proportion =  " [{0:.1f}%]".format(100 * (float(tool_metric)/tool_runtime))
                    #print("$"* 80)

                if baseline_metric != 0:
                    res_val = ", {0:.1f}%".format(100 *(float(tool_metric-baseline_metric) / baseline_metric))
                else:
                    res_val = "(N/A)"
                res_with_percentage = "{:,}".format(tool_metric) + res_val + proportion

                tool_res[bench_name][catagory][metric] = res_with_percentage
    print(tool_res)
    with open(final_res_file, 'w') as fd:
        json.dump(tool_res, fd, sort_keys=True, indent=4, ensure_ascii=False)
    print("writtne results to: %s" % final_res_file)
'''
    # loop through the benchmarks
    for bench_name in baseline_res:
        # generate security results
        baseline_sec = baseline_res[bench_name][iot2_settings.SECURITY_METRICS_TYPE]
        tool_sec = tool_res[bench_name][iot2_settings.SECURITY_METRICS_TYPE]
        tool_arr = []
        res_arr = []
        for sec_metric in iot2_settings.SECURITY_METRICS:
            baseline_val = baseline_sec[sec_metric]
            tool_val = tool_sec[sec_metric]
            # append value to array to write it over the bar
            tool_arr.append(tool_val)
            res_val = float(tool_val)/baseline_val
            res_arr.append(res_val)
            sec_dict.setdefault(sec_metric, []).append(res_val)
    print(sec_dict)
'''



def plot_figure(figure_name, figure_title, xtitle, ytitle, res_arr, absvals_arr, metrics_arr):
    # config width
    width = 0.2

    bars = metrics_arr
    fig, ax = plt.subplots(1, figsize=FIG_SIZE)
    idx = np.arange(len(bars))
    # plot the bars
    ax.barh(idx, res_arr, width, color='b', label='Baseline', align='center')
    ax.set_yticks(idx)
    ax.set_yticklabels(bars)
    ax.set_xlabel('Overhead (%)' % xtitle)
    for i, v in enumerate(res_arr):
        ax.text(v+0.05, i+0.05, "%.2f" %float(absvals_arr[i]), color='black', ha="center", va='top')
    ax.axvline(1, color='red')
    ax.text(1, 1.02, "Baseline", va='center', ha="center", bbox=dict(facecolor="w", alpha=0.5),
            transform=ax.get_xaxis_transform())
    plt.show()


def test():
    for name in iot2_settings.SECURITY_METRICS:
        print(name)
    security_metrics = iot2_settings.SECURITY_METRICS
    security_metrics.remove("TotalPriv_code")
    # setup paths
    baseline_file = iot2_settings.RESULTS_DIR_PATH + iot2_settings.IOT2_CONFIGS["BASELINE"] + \
                    iot2_settings.IOT2_CONFIGS["METRICS_RESULTS"] + \
                    iot2_settings.ALL_BENCHES_RESULT_FILE + iot2_settings.JSON_EXT
    tool_file = iot2_settings.RESULTS_DIR_PATH + iot2_settings.IOT2_CONFIGS["MYTOOL"] + \
                iot2_settings.IOT2_CONFIGS["METRICS_RESULTS"] + \
                iot2_settings.ALL_BENCHES_RESULT_FILE + iot2_settings.JSON_EXT

    with open(baseline_file, 'r') as b_fd:
        baseline_res = json.load(b_fd)
    with open(tool_file, 'r') as t_fd:
        tool_res = json.load(t_fd)
    for bench_name in baseline_res:
        print(bench_name)


#######################################################################
#                                 MAIN                                #
#######################################################################

#plot_benchmark('smart-locker', iot2_settings.METRICS_RESULTS_PATH)
#gen_all_bench_result_file(str(iot2_settings.METRICS_RESULTS_PATH))
#gen_final_results()
#get_metric_catagory()
#plot_results()
#plot_results()

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-bc', '--benchmark-config', dest='benchmark_configuration', type=str,
                        help="Flag to set the benchmark configuration to use from iot2_settings.BUILD_OPTIONS. if"
                             "not set, then all configurations are built."
                        , default=iot2_settings.ALL_STR)
    bc_options = ['mbed', 'mbed-os', 'secure_watchdog', 'secure_data_OS', 'secure_data_SDK', 'uvisor']

    args = parser.parse_args()
    result_option = []
    if args.benchmark_configuration != iot2_settings.ALL_STR:
        result_option.append(args.benchmark_configuration)
    elif args.benchmark_configuration:
        result_option.append(bc_options)
    print(result_option)

    for bench_config in result_option:
        static_metrics_cmd = ['python', 'iot2_measure_static_flash_and_ram.py', '-bc', str(bench_config), '-o']
        ret = subprocess.call(static_metrics_cmd,
                              stderr=subprocess.STDOUT)
        if ret != 0:
            print("[-] ERROR: could not collect static metrics")
        gen_all_bench_result_file(bench_config)
        if 'mbed' not in bench_config:
            gen_final_results(bench_config)
