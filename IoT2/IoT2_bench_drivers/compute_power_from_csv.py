import pandas
import os
import iot2_settings
import iot2_measure_static_flash_and_ram as iot2_helper
from collections import OrderedDict



MAX_NAME = "MAX"
MEAN_NAME = "MEAN"
MIN_NAME = "MIN"
ANALOG_PERIODS = "# of measurements"

def get_stats_for_active(power, periods):
    '''
        Computes stats on power for each entry in period 
    '''

    means = []
    mins = []
    #medians = []
    maxes = []
    for p in periods:
        means.append(power[p[0]:p[1]].mean())
        maxes.append(power[p[0]:p[1]].max())
        mins.append(power[p[0]:p[1]].min())
        #medians.append(power[p[0]:p[1]])

    average = pandas.Series(means).mean()
    maximum = max(maxes)
    minimum = min(mins)

    return  maximum, minimum, average


def get_indexes_for_active_periods(analog_time, periods):
    '''
        Returns the indexes of periods analog_time that most closely
        match those in active_periods
    '''

    p_idx = 0
    start_found = False
    active_times = []
    for idx, t in enumerate(analog_time):
        if not start_found and t >= periods[p_idx][0]:
            start_found = True
            start = idx
        elif start_found and t >= periods[p_idx][1]:
            end = idx
            start_found = False
            p_idx += 1
            active_times.append((start, end))
            if p_idx == len(periods):
                break
    return active_times


def get_active_periods(digital_time, digital):
    '''
        returns a list of tuples that are start and stop indexes in 
        analog_time when digital is high
    '''

    periods = []
    started = False
    for idx, value in enumerate(digital):
        try:
            v = int(value)
            if v == 1 and not started:
                start = float(digital_time[idx])
                started = True
            elif v == 0 and started:
                end = float(digital_time[idx])
                periods.append((start, end))
                start = None
                started = False
        except ValueError:
            #Reached end of data stop searching
            break

    return periods


if __name__ == '__main__':
    from argparse import ArgumentParser
    p = ArgumentParser()
    #p.add_argument("-c", "--capture", required=True,
    #               help="CSV of Salea Logic Pro Export")
    p.add_argument("--vcc_channel", default=1, type=int,
                   help="Column that contains VCC readings")
    p.add_argument("--current_channel", default=2, type=int,
                   help="Column that contains Current readings")
    p.add_argument("--digital_time_column", default=3, type=int,
                   help="Column that contains Digital Time Information")
    p.add_argument("--digital_sig", default=4, type=int,
                   help="Column that with digital information that delinated each run")
    p.add_argument("--gain", default=50.0, type=float,
                   help="Gain to convert current reading to AMPS")

    p.add_argument('-bc', '--benchmark-config', dest='benchmark_configuration', type=str,
                        help="Flag to set the benchmark configuration to use from iot2_settings.BUILD_OPTIONS. if"
                             "not set, then all configurations are built."
                        , default=iot2_settings.ALL_STR)



    args = p.parse_args()
    # path to metrics directory
    metrics_path = str(iot2_settings.RESULTS_DIR_PATH +
                       iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_TYPE] +
                       iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_CONFIG_RES_DIR] +
                       iot2_settings.METRICS_RESULTS_DIR)

    power_res_file = metrics_path + "power_results--"+ str(args.benchmark_configuration) +iot2_settings.CSV_EXT

    bench_name_list = []
    energy_csv_file_list = []
    power_result_dict = OrderedDict()
    power_result_dict[MAX_NAME] = []
    power_result_dict[MEAN_NAME] = []
    power_result_dict[MIN_NAME] = []
    power_result_dict[ANALOG_PERIODS] = []

    energy_csv_file_list = iot2_helper.gen_filelist(metrics_path, energy_csv_file_list, iot2_settings.CSV_EXT)
    for capture_file in energy_csv_file_list:
        
        bench_name = iot2_helper.get_filename(capture_file)
        bench_name_list.append(bench_name)

        data = pandas.read_csv(capture_file)#args.capture)
        vcc = data.get(data.columns[args.vcc_channel])
        mA = data.get(data.columns[args.current_channel]) / args.gain
        analog_time =  data.get(data.columns[0])
        digital_time = data.get(data.columns[args.digital_time_column])
        digital_sig = data.get(data.columns[args.digital_sig])

        sample_period = analog_time[1]-analog_time[0]

        power = vcc * mA

        digital_periods = get_active_periods(digital_time, digital_sig)
        analog_periods = get_indexes_for_active_periods(analog_time, digital_periods)
        maximum, minimum, average = get_stats_for_active(power, analog_periods) 

        # update power results dictionary
        power_result_dict[MAX_NAME].append(maximum)
        power_result_dict[MEAN_NAME].append(average)
        power_result_dict[MIN_NAME].append(minimum)
        power_result_dict[ANALOG_PERIODS].append(len(analog_periods))
        
        #filename = os.path.split(capture_file)[-1].strip()
        #print('%s, %f, %f, %f, %i'%(filename,maximum,  average, minimum, len(analog_periods) ))
    
    power_df = pandas.DataFrame(power_result_dict, index=bench_name_list)

    power_df.to_csv(power_res_file)
    print("[+] Power measurment for [%s] benchmarks have been written to: %s"
        % (args.benchmark_configuration, power_res_file))