import os
import sys
import argparse
import numpy
import json
import subprocess
import errno
import iot2_settings

#######################################################################
#                              GLOBALS                                #
#######################################################################


CURR_PATH = os.path.dirname(os.path.realpath(__file__))
PROJ_ROOT_DIR = os.path.abspath('../../')
IoT2_EVAL_CONFIG_FILE = PROJ_ROOT_DIR + "/IoT2/IoT2-Eval-Config/evaluation-results-config.json"
IoT2_METRIC_CONFIG_FILE = PROJ_ROOT_DIR + "/IoT2/IoT2-Eval-Config/eval-metric-config.json"
IoT2_RESULTS_PATH = PROJ_ROOT_DIR
STATIC_FLASH_RAM_METRICS_FILENAME = "FLASH_STATIC_RAM_METRICS.csv"

SIZE_CMD = 'arm-none-eabi-size -A -x '
SIZE_EXT = '.size'

BIN_EXT = '.elf'

ROP_GADGETS_COMPILER = PROJ_ROOT_DIR + "/tools/ROPgadget/ROPgadget.py"
ROP_GADGETS_COMPILER_OPTIONS = " --thumb --binary "
ROP_GADGETS_CMD = "python3 " + ROP_GADGETS_COMPILER + ROP_GADGETS_COMPILER_OPTIONS
ROP_EXT = '.ROPresiliency'

OBJDUMP_CMD = "arm-none-eabi-objdump -d "
OBJDUMP_EXT = ".objdump"

RAM_TYPE = "RAM"
FLASH_TYPE = "FLASH"
OTHER_MEM_TYPE = "OTHER"
SIZE_INFO_LENGTH = 3           # size utility outputs 3 values (region name, size, and addr)


METRIC_RESULTS_ROWS = ["Bench",
                       "Total Flash", "Min Flash Region", "Max Flash Region", "Ave Flash Region", "# Flash Regions",

                       "Total RAM", "Min RAM Region", "Max RAM Region", "Ave RAM Region", "# RAM Regions",

                       "Total Exec Code Regions", "Min Exec Code Regions", "Max Exec Code Regions",
                       "Ave Exec Code Regions", "# Exec Code Regions",

                       "Total ROP Gadgets", "Min ROP Gadgets", "Max ROP Gadgets",
                       "Ave ROP Gadgets", "Min ROP Gadgets Region Name", "Max ROP Gadgets Region Name"

                       "Total Other Regions", "Min Other Regions", "Max Other Regions", "Ave Other Regions",
                       "# Other Regions"
                       ]


#######################################################################
#                              CLASSES                                #
#######################################################################

class FlashMemRegion:
    def __init__(self, name, size, addr):
        self.name = name
        self.size = size
        self.start_addr = addr
        self.end_addr = self.start_addr + self.size
        self.type = get_region_type(self.start_addr)
        self.num_rops = 0

    def is_exec_code_reg(self):
        """
        Checks if the region is an executable code region
        :return: True if the region is an executable code region
        """
        global EXEC_CODE_REGS
        if any(reg_id in self.name for reg_id in EXEC_CODE_REGS):
            return True

        return False

    def get_name(self):
        return self.name


class RegionMetrics:
    def __init__(self, name):
        self.name = name
        self.regs_list = []
        self.regs_size_list = []
        self.num_rops_list = []
        self.sum_regs = 0
        self.min_reg = 0
        self.max_reg = 0
        self.ave_reg = 0
        self.num_regs = 0
        self.min_reg_name = ""
        self.max_reg_name = ""
        self.sum_rops = 0
        self.min_rops = 0
        self.max_rops = 0
        self.ave_rops = 0
        self.unknown_region_rop = 0
        self.min_rop_reg_name = "NA"
        self.max_rop_reg_name = "NA"
        self.csv_file_results = []

    def calc_rop_metrics(self):
        # append every region's rop gadgets to a list
        for exec_reg in self.regs_list:
            self.num_rops_list.append(exec_reg.num_rops)
        # append any unknown region rop if it exists
        if self.unknown_region_rop:
            self.num_rops_list.append(self.unknown_region_rop)
        # calculate metrics
        self.min_rops = numpy.min(self.num_rops_list)
        self.max_rops = numpy.max(self.num_rops_list)
        self.ave_rops = numpy.mean(self.num_rops_list)
        self.sum_rops = numpy.sum(self.num_rops_list)
        self.min_rop_reg_name = self.regs_list[self.num_rops_list.index(self.min_rops)].name
        self.min_rop_reg_name = self.regs_list[self.num_rops_list.index(self.max_rops)].name

    def calc_metrics(self):

        # check if the list is not empty
        if self.regs_size_list:
            self.min_reg = numpy.min(self.regs_size_list)
            self.max_reg = numpy.max(self.regs_size_list)
            self.ave_reg = numpy.mean(self.regs_size_list)
            self.sum_regs = numpy.sum(self.regs_size_list)
            self.num_regs = len(self.regs_size_list)
            self.get_min_max_reg_name()
        # in case the list is empty, return 0 and NA
        else:
            self.min_reg = 0
            self.max_reg = 0
            self.ave_reg = 0
            self.sum_regs = 0
            self.num_regs = 0
            self.min_reg_name = "NA"
            self.max_reg_name = "NA"
        # put the results in the csv file list
        self.get_csv_file_results()
        return

    def get_min_max_reg_name(self):
        for mem_reg in self.regs_list:
            if mem_reg.size == self.min_reg:
                self.min_reg_name = mem_reg.name
            elif mem_reg.size == self.max_reg_name:
                self.max_reg_name = mem_reg.name
            # if we already found min and max, then there is no need to continue searching
            if self.min_reg_name and self.max_reg_name:
                return

    def get_csv_file_results(self):
        self.csv_file_results.append(str(self.sum_regs))
        self.csv_file_results.append(str(self.min_reg))
        self.csv_file_results.append(str(self.max_reg))
        self.csv_file_results.append(str(self.ave_reg))
        self.csv_file_results.append(str(self.num_regs))
        # this should execute only when collecting  exec_regions (i.e., ROP gadget results)
        if self.num_rops_list:
            self.csv_file_results.append(str(self.sum_rops))
            self.csv_file_results.append(str(self.min_rops))
            self.csv_file_results.append(str(self.max_rops))
            self.csv_file_results.append(str(self.ave_rops))
            self.csv_file_results.append(str(self.min_rop_reg_name))
            self.csv_file_results.append(str(self.max_rop_reg_name))


class FlashStaticMemMetrics:
    def __init__(self, name):
        self.name = name
        self.flash_metrics = RegionMetrics(name)
        self.ram_static_metrics = RegionMetrics(name)
        self.other_mem_metrics = RegionMetrics(name)
        self.exec_code_metrics = RegionMetrics(name)
        self.bench_csv_results = [self.name]
        self.isolated_data_regs_size = 0

    def get_bench_csv_results(self):
        # calculate the metrics
        self.flash_metrics.calc_metrics()
        self.ram_static_metrics.calc_metrics()
        self.exec_code_metrics.calc_metrics()
        self.other_mem_metrics.calc_metrics()
        # append the metrics to bench_csv_results
        self.bench_csv_results.append(self.flash_metrics.csv_file_results)
        self.bench_csv_results.append(self.ram_static_metrics.csv_file_results)
        self.bench_csv_results.append(self.exec_code_metrics.csv_file_results)
        self.bench_csv_results.append(self.other_mem_metrics.csv_file_results)
        '''
        [iot2-debug]: added this to write the results to the JSON file. OK to leave but should
        be cleaned in the future
        '''
        total_flash = self.flash_metrics.sum_regs
        total_ram = self.ram_static_metrics.sum_regs
        total_exec_code = self.exec_code_metrics.sum_regs
        total_rop_gadgets = self.exec_code_metrics.sum_rops
        max_code_ratio = (float(self.exec_code_metrics.max_reg) / self.exec_code_metrics.sum_regs) * 100
        max_data_ratio = (float(self.ram_static_metrics.sum_regs - self.isolated_data_regs_size)/ self.ram_static_metrics.sum_regs) * 100
        return self.bench_csv_results, total_flash, total_ram, total_exec_code, total_rop_gadgets, max_code_ratio, max_data_ratio

    def check_isolated_data_region(self, mem_reg):
        global ISOLATED_DATA_REGIONS
        if any(reg_id in mem_reg.name for reg_id in ISOLATED_DATA_REGIONS):
            self.isolated_data_regs_size += mem_reg.size
        return

def get_flash_ram_static_metrics(size_file_list, rop_file_list,  metric_result_path):
    global SIZE_INFO_LENGTH, FLASH_TYPE, RAM_TYPE, OTHER_MEM_TYPE, METRIC_RESULTS_ROWS, BENCHMARK_CONFIG_NAME

    sizefile_num_lines_skip = 2  # First 2 lines do not contain size information in the

    # create a result list and append the table titles to it. This list will hold all the results and is written
    # to the csv file in the end.
    csv_results_list = [METRIC_RESULTS_ROWS]

    for size_file, rop_file in zip(size_file_list, rop_file_list):
        sizefile_line_num = 0       # reset the line #
        fsmm = FlashStaticMemMetrics(get_filename(size_file))
        with open(size_file, 'r') as fd:
            size_fd = fd.readlines()
            size_fd = [line.rstrip(' \r\t\n\0') for line in size_fd]

            for line in size_fd:
                line = line.split()
                # check that the line contain information we are interested in and not file name or file header info
                if len(line) != SIZE_INFO_LENGTH or sizefile_line_num < sizefile_num_lines_skip:
                    sizefile_line_num += 1
                    continue

                # create an object of the current region
                mem_region = FlashMemRegion(line[0], int(line[1], 16), int(line[2], 16))
                # put the region in its type
                if mem_region.type == FLASH_TYPE:
                    fsmm.flash_metrics.regs_list.append(mem_region)
                    fsmm.flash_metrics.regs_size_list.append(mem_region.size)
                elif mem_region.type == RAM_TYPE and mem_region.name != '.heap': # execlude heap if it is added
                    fsmm.ram_static_metrics.regs_list.append(mem_region)
                    fsmm.ram_static_metrics.regs_size_list.append(mem_region.size)
                    fsmm.check_isolated_data_region(mem_region)
                else:
                    fsmm.other_mem_metrics.regs_list.append(mem_region)
                    fsmm.other_mem_metrics.regs_size_list.append(mem_region.size)

                # check if the region is declared as an executable code region
                if mem_region.is_exec_code_reg():
                    fsmm.exec_code_metrics.regs_list.append(mem_region)
                    fsmm.exec_code_metrics.regs_size_list.append(mem_region.size)

            # calculate ROP gadgets for the benchmark, we are only using executable regions
            fsmm = get_rop_gadgets_metrics(rop_file, fsmm)

            # calculate the metrics for the regions
            bench_csv_results, json_flash, json_ram, json_exec_code,json_rops, json_max_code_ratio, json_max_data_ratio = fsmm.get_bench_csv_results()

            #----------------------------------------------------------------------------------------------
            '''
            [iot2-debug]: added the following to write the static results to the json results file. This is
            a non-optimal hack and should be cleaned in the future.
            '''
            write_results_to_json_file(json_flash, json_ram, json_exec_code, json_rops,
                                       json_max_code_ratio, json_max_data_ratio, size_file)
            #----------------------------------------------------------------------------------------------

            # append the results to the csv result list
            csv_results_list.append(bench_csv_results)

    # write the results of this benchmark binary to the results file
    write_flash_mem_static_metrics_result_file(metric_result_path, csv_results_list)
    return


def get_rop_gadgets_metrics(rop_file, fsmm):
    global SIZE_INFO_LENGTH, FLASH_TYPE, RAM_TYPE, OTHER_MEM_TYPE, METRIC_RESULTS_ROWS

    # create a result list and append the table titles to it. This list will hold all the results and is written
    # to the csv file in the end.)
    with open(rop_file, 'r') as fd:
        rop_fd = fd.readlines()
        rop_fd = [line.rstrip(' \r\t\n\0') for line in rop_fd]

        for line in rop_fd:
            line = line.split()
            # check that the line contain information we are interested in and not file name or file header info
            if not line or '0x' not in line[0]:
                continue

            gadget_addr = int(line[0], 16)  # line[0] is the address of the gadget in <.ROPresiliency> file
            # check which region the gadget belongs to, if not add the gadget to the unknown region
            gadget_reg_found = False
            for exec_reg in fsmm.exec_code_metrics.regs_list:
                if exec_reg.start_addr <= gadget_addr <= exec_reg.end_addr:
                    exec_reg.num_rops += 1
                    gadget_reg_found = True

            # the region was not found, perhaps an error from the ROP compiler or miss-configured EXEC_REGS
            if not gadget_reg_found:
                fsmm.exec_code_metrics.unknown_region_rop += 1

        # collect and calculate the statistics for rop gadgets in the benchmark
        fsmm.exec_code_metrics.calc_rop_metrics()
    return fsmm


def write_results_to_json_file(flash_size, ram_size, exec_code_size, rop_gadgets, max_code_ratio, max_data_ratio, size_file):
    global BENCHMARK_CONFIG_NAME
    bench_name = get_filename(size_file)
    # [iot2-debug] ugly hack, remove mytool from benchname-------------------------
    if '--' in bench_name:
        bench_name = bench_name.split('--')[0]
    #------------------------------------------------------------------------------
    bench_json_result_file = str(iot2_settings.RESULTS_DIR_PATH +
                                 iot2_settings.BUILD_OPTIONS[BENCHMARK_CONFIG_NAME][iot2_settings.BENCH_TYPE] +
                                 iot2_settings.BUILD_OPTIONS[BENCHMARK_CONFIG_NAME][iot2_settings.BENCH_CONFIG_RES_DIR] +
                                 iot2_settings.METRICS_RESULTS_DIR) + bench_name + iot2_settings.JSON_EXT #iot2_settings.METRICS_RESULTS_PATH + bench_name + iot2_settings.JSON_EXT
    num_indrect_calls = get_num_indirect_calls(size_file)
    with open(bench_json_result_file, 'r') as fd:
        json_results = json.load(fd)

    # read_stack results to add them to ram
    stack_heap = json_results["Stack_heap_usage"]
    # update json results
    json_results["Total_FLASH"] = flash_size
    json_results["Total_RAM"] = ram_size + stack_heap
    json_results["Executable_code"] = exec_code_size
    json_results["Max_code_ratio"] = max_code_ratio
    json_results["Max_data_ratio"] = max_data_ratio
    json_results["ROP_gadgets"] = rop_gadgets
    json_results["Indirect_calls"] = num_indrect_calls
    # write the updated results to the file
    with open(bench_json_result_file, "w") as fd:
        json.dump(json_results, fd, sort_keys=True, indent=4, ensure_ascii=False)
    print("-" * 80)
    print("[+] Benchmark static results written to JSON file at: %s" % bench_json_result_file)
    print("-" * 80)
    return


def get_num_indirect_calls(size_file):
    # objdump is in the same directory of the size file, so just replace the extension
    objdump_file = size_file.replace(SIZE_EXT, OBJDUMP_EXT)
    print("*"*80)
    print(objdump_file)

    branch_inst_idx = 2
    num_indrect_calls = 0
    check_regs_flag = False     # set after validating the branch instruction is for indirect calls
    with open(objdump_file, 'r') as fd:
        lines = fd.readlines()
        for line in lines:
            line = line.split()
            if len(line) >= (branch_inst_idx+1):
                # instructions with indirect calls are either BX or BLX, these however can be also
                # conditional branches (e.g. BXNE) so we check (BX,BLX) and that the used target is
                # not LR to avoid return calls
                instruction = line[branch_inst_idx]
                if 1 < len(instruction) < 3 :
                    if instruction[0] == 'b' and instruction[1] == 'x':
                        check_regs_flag = True
                elif len(instruction) >= 3:
                    if instruction[0] == 'b' and instruction[1] == 'x':
                        check_regs_flag = True
                    elif instruction[0] == 'b' and instruction[1] == 'l' and instruction[2] == 'x':
                        check_regs_flag = True
                # check register to validate it is not LR
                if check_regs_flag:
                    # register after the instruction
                    if line[branch_inst_idx+1][0] == 'r' and line[branch_inst_idx+1][1].isdigit():
                        num_indrect_calls += 1
                    # reset check flag
                    check_regs_flag = False
    # return the final number of indirect calls
    print("indirect calls = %d" % num_indrect_calls)
    print("*"*80)
    return num_indrect_calls

#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def gen_filelist(file_list_path, file_list, file_ext):
    """
    Returns a list of files with the given extension for the given directory
    :param file_list_path: Path to the files
    :param file_list: Variable to hold the list of file names (i.e. the whole path)
    :param file_ext: The extension of the files
    :return: List of files with the given extension
    """
    # check if path exists
    if not os.path.exists(file_list_path):
        # path does not exist, print a note and return the same file list
        print("[-] Directory  <%s> does not exist, returning empty file list" % file_list_path)
        return file_list

    for filename in os.listdir(file_list_path):
        if filename.endswith(file_ext):
            file_list.append(os.path.join(file_list_path, filename))
    # sort the file list before returning
    file_list = sorted(file_list, key=str.lower)
    return file_list


def dump_analysis_files(file_list, analysis_path, analysis_cmd, file_ext):
    """
    Creates and writes the information of the given file list to the analysis path using the configured
    command and file extension. The resulting files will all have the given extension with the same file name from
    the file list.
    :param file_list: List of binary files
    :param analysis_path: Path where the resulting analysis information files will be written to.
    :param analysis_cmd: Command to run on each file of the binary file list.
    :param file_ext: Extension to be used for the resulting files.
    :return: None.
    """
    try:

        for bin_file in file_list:
            analysis_filename = analysis_path + get_filename(bin_file) + file_ext
            subprocess.call(analysis_cmd + bin_file + ">" + analysis_filename, shell=True)
        print("[+] Done generating <%s> files" % file_ext)
        return
    except OSError:
        raise


def get_filename(file_fullpath):
    """
    Returns the filename without the full path
    :param file_fullpath:
    :return: Returns the filename
    """
    filename = file_fullpath.split("/")[-1].split(".")[0]
    return filename


def get_region_type(addr):
    global FLASH_TYPE, RAM_TYPE,OTHER_MEM_TYPE, FLASH_START, FLASH_END, RAM_START, RAM_END
    if FLASH_START <= addr <= FLASH_END:
        return FLASH_TYPE
    elif RAM_START <= addr <= RAM_END:
        return RAM_TYPE
    else:
        return OTHER_MEM_TYPE


def write_flash_mem_static_metrics_result_file(metric_result_path, csv_result_list):
    global STATIC_FLASH_RAM_METRICS_FILENAME
    metric_results_file = metric_result_path + STATIC_FLASH_RAM_METRICS_FILENAME
    with open(metric_results_file, 'w') as mrf:
        # write the table rows
        for res_row in csv_result_list:
            res_row = merge_csv_result_row(res_row)
            res_row = ",".join(res_row) + "\n"
            mrf.write(res_row)


def merge_csv_result_row(res_row):
    merged_row = []
    for res_list in res_row:
        if isinstance(res_list, list):
            merged_row.extend(res_list)
        else:
            merged_row.append(res_list)
    return merged_row


#######################################################################
#                                 MAIN                                #
#######################################################################

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--binpath', dest='BIN_PATH', type=str,
                        help="Flag to set the path of the binaries directory", required=False)

    parser.add_argument('-a', '--analysispath', dest='ANALYSIS_PATH', type=str,
                        help="Flag to set the output path of the analysis directory", required=False)

    parser.add_argument('-r', '--metricspath', dest='METRICS_PATH', type=str,
                        help="Flag to set the path of the metric results directory", required=False)
    parser.add_argument('-bc', '--benchmark-config', dest='benchmark_configuration', type=str,
                        help="Flag to set the benchmark configuration to use from iot2_settings.BUILD_OPTIONS. if"
                             "not set, then all configurations are built."
                        , default=iot2_settings.ALL_STR)
    parser.add_argument('-o', '--objdump', dest='dump_objdump', default=False,
                        action='store_true', help="Flag dump binaries of the benchmarks to get the # of indirect calls")

    args = parser.parse_args()

    metrics_path = str(iot2_settings.RESULTS_DIR_PATH +
                       iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_TYPE] +
                       iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_CONFIG_RES_DIR] +
                       iot2_settings.METRICS_RESULTS_DIR)

    analysis_path = str(iot2_settings.RESULTS_DIR_PATH +
                        iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_TYPE] +
                        iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_CONFIG_RES_DIR] +
                        iot2_settings.ANALYSIS_FILES_DIR)

    bins_path = str(iot2_settings.RESULTS_DIR_PATH +
                        iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_TYPE] +
                        iot2_settings.BUILD_OPTIONS[args.benchmark_configuration][iot2_settings.BENCH_CONFIG_RES_DIR] +
                        iot2_settings.BINS_DIR)

    # setup the paths here statically, control these from the json file
    #args.ANALYSIS_PATH = str(iot2_settings.ANALYSIS_RESULTS_PATH)
    #args.BIN_PATH = str(iot2_settings.BINS_PATH)
    #args.METRICS_PATH = str(iot2_settings.METRICS_RESULTS_PATH)

    BENCHMARK_CONFIG_NAME = args.benchmark_configuration
    # read metrics configuration from json file
    metric_config = json.load(open(IoT2_METRIC_CONFIG_FILE))
    FLASH_START = int(metric_config["FLASH_METRICS_SETUP"]["START_ADDR"], 16)
    FLASH_END = int(metric_config["FLASH_METRICS_SETUP"]["END_ADDR"], 16)
    RAM_START = int(metric_config["RAM_METRICS_SETUP"]["START_ADDR"], 16)
    RAM_END = int(metric_config["RAM_METRICS_SETUP"]["END_ADDR"], 16)
    EXEC_CODE_REGS = metric_config["EXEC_CODE_REGIONS"]
    ISOLATED_DATA_REGIONS = metric_config["ISOLATED_DATA_REGIONS"]

    # generate a list of the binary files to analyze
    bin_filelist = []
    bin_filelist = gen_filelist(bins_path, bin_filelist, BIN_EXT)

    # Dump the size information of the binary files
    dump_analysis_files(bin_filelist, analysis_path, SIZE_CMD, SIZE_EXT)
    # dump rop gadgets analysis files
    dump_analysis_files(bin_filelist, analysis_path, ROP_GADGETS_CMD, ROP_EXT)
    # dump objdump of binaries
    if args.dump_objdump:
        dump_analysis_files(bin_filelist, analysis_path, OBJDUMP_CMD, OBJDUMP_EXT)

    # get a list of size files
    size_filelist = []
    size_filelist = gen_filelist(analysis_path, size_filelist, SIZE_EXT)
    # get a list of ROPresiliency files
    rop_filelist = []
    rop_filelist = gen_filelist(analysis_path, rop_filelist, ROP_EXT)

    # calculate the results and write it to a csv file
    get_flash_ram_static_metrics(size_filelist, rop_filelist, metrics_path)
    print("[+] DONE! results have been written to file.")


