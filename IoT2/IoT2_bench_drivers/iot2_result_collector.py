import os
import sys
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


#######################################################################
#                              GLOBALS                                #
#######################################################################

# iot2 special msgs for result collection
IOT2_BEGIN_BENCHMARK = "[IoT2] benchmark: START"    # this message is needed to set an event and start the tcp driver
IOT2_START_COLLECTING_RESULTS_MSG = "[IoT2] collect_results: START"
IOT2_END_COLLECTING_RESULTS_MSG = "[IoT2] collect_results: END"


#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def set_bench_event(bench_event):
    bench_event.set()
    return


def collect_benchmark_results(ser, metric_dict, iteration):
    ser_input = ser.readline()
    ser_input = ser_input.rstrip('\r\n')
    # read metrics until end msg is sent
    while ser_input != IOT2_END_COLLECTING_RESULTS_MSG:

        metric_result = ser_input.split("->")
        metric_name, metric_values = metric_result[1].split(":")
        #print("-"*80)
        #print(metric_name)
        #print("-" * 80)
        #print(metric_values)
        #print("-"*80)
        submetrics = metric_values.split(",")
        # if there is only a single value, just store it in the dictionay directly
        if len(submetrics) == 1:
            # there is only 1 value which is the metric result
            # handle the priv code calculation
            if metric_name == "TotalPriv_code":
                metric_val = int(pow(2, int(float(submetrics[0])))/float(1024))
            # all other metrics
            else:
                metric_val = int(float(submetrics[0]))
            metric_dict.setdefault(metric_name, []).append(metric_val)
        # there are multiple submetrics, add the recursively to the dictionary
        else:
            # check if this is the first iteration or not
            if metric_name in metric_dict.keys():
                submetric_dict = metric_dict[metric_name]
            else:
                # create dictionary if this is the first iteration
                submetric_dict = OrderedDict()
            # add submetrics to its own dictionary
            for submetric in submetrics:
                submetric_name, submetric_val = submetric.split("=")
                submetric_val = int(submetric_val, 10)
                submetric_dict.setdefault(submetric_name, []).append(submetric_val)

            # add all to the submetrics to global metric dictionary
            metric_dict[metric_name] = submetric_dict

        # receive next result
        ser_input = ser.readline()
        ser_input = ser_input.rstrip('\r\n')
    return metric_dict


def iot2_serial_collector(user_port, iterations, bench_config_name, benchmark_name, append_results, iot2_synch_obj=None):
    global IOT2_BEGIN_BENCHMARK

    res_file = str(iot2_settings.RESULTS_DIR_PATH +
                   iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_TYPE] +
                   iot2_settings.BUILD_OPTIONS[bench_config_name][iot2_settings.BENCH_CONFIG_RES_DIR] +
                   iot2_settings.METRICS_RESULTS_DIR) + benchmark_name + iot2_settings.JSON_EXT
    if iot2_settings.OS_BENCHMARKS == 0:
        IOT2_BEGIN_BENCHMARK = "Welcome to IoT2"
    try:
        ser = serial.Serial(port=user_port, baudrate=9600, dsrdtr=0, rtscts=0)
        ser.flushInput()
        ser.flushOutput()
        iterations_cntr = 0
        print(res_file)
        # check if we should append or create new file
        if append_results:
            with open(res_file, 'r') as append_fd:
                metric_dict = json.load(append_fd)
        else:
            metric_dict = OrderedDict()

        while iterations_cntr < iterations:
            ser.flushInput()
            ser.flushOutput()
            print("-"*80)
            print("ITERATION (%d/%d) for BENCHMARK: %s" % (iterations_cntr, iterations, benchmark_name))
            print("-"*80)
            # reset ser_input
            ser_input = ""
            t_arr = []
            # benchmark might use the serial, keep receiving messages until result collection starts
            while IOT2_START_COLLECTING_RESULTS_MSG not in ser_input:
                ser_input = ser.readline()
                ser_input = ser_input.rstrip('\r\n')
                ser_input = ''.join([i if ord(i) < 128 else '' for i in ser_input])
                # if benchmark started, set the signal for the tcp process to start
                # used the below solution to avoid serial bugs
                if IOT2_BEGIN_BENCHMARK in ser_input:
                    print("BENCHMARK STATUS IS SET")
                    if iot2_synch_obj:
                        iot2_synch_obj.set_benchmark_status()
                # check if tcp process started, if se reset benchmark status for the next iteration
                if iot2_synch_obj:
                    if iot2_synch_obj.get_tcp_proc_flag() == 1 and iot2_synch_obj.get_benchmark_status() == 1:
                        iot2_synch_obj.reset_benchmark_status()
                        print("BENCHMARK STATUS RESET AFTER TCP FLAG IS SET")
                m_seconds = "%.4f" % (time.time())
                input_time = "%s.%s" % (datetime.now().strftime('%H:%M:%S'), m_seconds[-4:])
                print("%s,[IoT2]>> %s" % (input_time, ser_input))

            # start recording the received results
            metric_dict = collect_benchmark_results(ser, metric_dict, iterations_cntr)
            print("-"*80)
            print("[*] serial runner finished, closing...")
            print("-"*80)

            # update counter
            iterations_cntr += 1
        #print("#"*80)
        #print(metric_dict)
        # write results to a file
        with open(res_file, 'w') as fd:
            json.dump(metric_dict, fd, sort_keys=True, indent=4, ensure_ascii=False)
        #print("#"*80)
        print("="*80)
        print("[+] Results written to %s" % res_file)
        print("="*80)
        return
    except KeyboardInterrupt:
        print("[-] CTRL-C pressed...")

    ser.close()
    print("[IoT2]>> Closing....")
    return


#######################################################################
#                                 MAIN                                #
#######################################################################
#iot2_serial_collector('/dev/ttyACM0', 1)
