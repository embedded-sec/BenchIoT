import pandas
import os


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
    p.add_argument("-c", "--capture", required=True,
                   help="CSV of Salea Logic Pro Export")
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

    args = p.parse_args()
    data = pandas.read_csv(args.capture)
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
    
    filename = os.path.split(args.capture)[-1].strip()
    print('%s, %f, %f, %f, %i'%(filename,maximum,  average, minimum, len(analog_periods) ))

