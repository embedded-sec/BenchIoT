# BenchIoT: A security benchmark for the Internet of Things

This is a joint research effort between Purdue's [HexHive](http://hexhive.github.io/) and [DCSL](https://engineering.purdue.edu/dcsl/) research groups.  The [paper](https://nebelwelt.net/publications/files/19DSN.pdf) was presented at [the 49th IEEE/IFIP International Conference on Dependable Systems and Networks](http://2019.dsn.org/). 

Both have many more open sourced software:
*  [HexHive Software](https://github.com/HexHive)
*  [DCSL Software](https://github.com/purdue-dcsl)



## Prerequisites

You need to have the following setup before using BenchIoT.

- The GNU ARM embedded toolchain (https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads).
- make
- GDB-with python pulgin (for debugging script only)
- OpenOCD running with your board.(http://openocd.org/)
- mbed-cli (https://github.com/ARMmbed/mbed-cli)

## Project Strutcure


```
|-> BenchIoT dir
	|-> IoT2_MAKEFILES (These are modular makefiles to build each part separatly (e.g., mbed-os))
	|-> IoT2 (This contains the evaluation library files and scripts, also mbed-os instrumented files) check the explanation below for details
	|-> benchmarks (Ones with an OS should be in mbed-os-benchmarks. Ones without an OS should be in mbed-benchmarks)
	|-> lib (Any library that is not part of the core  mbed-SDK/OS. The IoT^2 library resides here. Also any board specific library will be here.)
	|-> os-lib (Contains mbed-sdk, mbed-os)
	|-> support-scripts (Contatins gdb-python debugging script, pyterm as a PC terminal for the serial port, and a simple tcp-client to test Ethernet applicatoins.)
	|-> template (Shows a simple blinky example application for each type (i.e. with/without an OS)
```

## Setup

Generally, running the setup.sh script should be enough to setup the symlinks:

```
./setup.sh
```

This file does 2 things:

First, it sets up IoT2 symlinks and instrumented mbed/mbed-os files.

Second, since we are supporting both mbed-sdk (baremetal) and mbed-os, it would be
 better to have a consistent way of adding any extra library (e.g. FATFS library). 
Many of these however are a part of the mbed-os structure, listed under "FEATURES". 
Thus, all of these will be accessed from <lib/mbed-libs/FEATURES> using symlinks.



## Quick Start to build an application

**Note that (IoT2/iot2) is used here as a shortcut for BenchIoT files, script, and functions. It will be replaced soon.**

See the list of options below and use the one that works for you. I will be using EVAL_F469NI as the board for the examples below. The same command should run when you change 
that to your board. (e.g., DISCO_F407VG, K64F...etc)

**Baseline benchmark**
```
# this creats a makefile in the application directory, you can change iot2_debug
# to release if you want to build with -Os
mbed export -m EVAL_F469NI -i GCC_ARM --profile iot2_debug
# build the benchmark
make -j4 BOARD=EVAL_F469NI IOT2_ALL=1 CUSTOM_BIN_SUFFIX=--baseline iot2benchmark
# A binary should be built in both the (bins & BUILD) directories. Now you can run
# the benchmark using gdb
<YOUR_GDB> <BINARY_FILE> -x support_scripts/gdb_helpers.py
# If your benchmark is using the serial, you might need to run pyterm to see
# the output throught the terminal. You should find a copy in support_scripts
sudo python pyterm.py -p <YOUR_PORT> -b <BAUD_RATE>
```
Below is a brief explanation of the above:

- CUSTOM_BIN_SUFFIX: This just adds your customized suffix to the name of binary.
- IOT2_ALL: Sets the macros to enable the iot2 runtime library.
- iot2benchmark: Compile and links IoT2Lib with application. You can use <all> instead if you do not use any of the iot2 runtime library features.
- YOUR_GDB: Your arm-none-eabi-gdb.
- BINARY_FILE: This the <.elf> file you want to run.
- YOUR_PORT: After you connect your board do a `ls /dev/tty*`, and see if there is a new port for the board. Usually this is either /dev/ttyUSB0 or /dev/ttyACM0
- BAUD_RATE: This is the speed of communication, usually 9600. Check the application to verify.



## How to build an application? How to enable IoT2? (Details)

Check the list for a quick start above. For a detailed explanation continue reading. If anything is not clear, check IOT2_MAKEFILES/IOT2LIB.mk and Benchmark.mk in the benchmark directory. These 
are the only files used in addtion to the makefile.

First, use mbed-cli to create the makefile according to your benchmark/board. Wether you would like to build it with debug/release options.

Then, you build an application with the desired configuration. In general you will run the following commands inside the benchmark directory:

```
mbed export -m <BOARD> -i <BUILD_TYPE/COMPILER> --profile <COMPILE_CONFIGS>
make BOARD=<BOARD> IOT2_ALL=<1 enables IOT2 measurements> IOT2_CONFIG=<0,1,2, or 3> CUSTOM_LINKER=<LINKER_OPTION> CUSTOM_BIN_SUFFIX=--<SUFFIX_FOR_BINARY_NAME> iot2benchmark
```


Below is an explanation for the different flags to use (usually enabled with =1):

IoT2: The flag IoT2 links the IoT2 runtime library, but does not collect metrics
 related to the vector table (e.g., Privileged instructions).

IOT2_ISR_RUNTIME: This flags enables tracking all IoT2 metrics, including the vector table metrics.

IOT2_RESULTS_COLLECTOR: This enables sending the results at the end of the benchmark.
Otherwise, it will just be an empty function.

IOT2_ALL: enables all the above three options automatically.

IOT2_CONFIG: Specifies which vector table to use from IoT2-lib/IoT2_Config.c, if not used it defaults to to 0 (check IOT2LIB.mk for details)

CUSTOM_LINKER: Choose which linker file to use. The value here should exactly match the suffix after a double dash (--<LINKER_OPTION>). Check the EVAL_F469NI directory for details. If nothing is specified then the default linker is used.

CUSTOM_BIN_SUFFIX: The suffix you would like to have for the compiled binary.

## What is  IoT2_Config.h?

To avoid modifying the source code for each board, a generic IoT2 macros are used. 
So, once another board is added, only this configuratio file is modified.

For example:

```
#define IoT2_USART1_IRQ         USART1_IRQn
```

The IoT2 application uses IoT2_USART1_IRQ as the interrupt ID for UART. For the 
EVAL_F469NI board this corresponds to USART1_IRQn. If you are using another board
you should define the same macro according to your board UART ID.

Just copy one of the available configurations in the file and modify it to your
board. Note the the name of the board has to match the one defined by mbed-cli.

## How to run you application manually?

After building the application, the binary will be at build/<YOUR_BOARD> directory. 
You can run the benchmark manually using the scripts at the **<support_scripts>**
directory.You can use gdb_helpers for the debugger and pyterm for the serial terminal.

For the benchmark TCP driver, you can find the TCP driver for each benchmarks at 
**<IoT2/IoT2-bench-drivers/benchmark_TCP_drivers>**. For example, if you are
running smart_locker, then you will run 

```
python IoT2/IoT2-bench-drivers/smart_locker.py
```



## How to run the benchmark(s) automatically?

The way the evaluation works is by copying the binaries from the project then
running the benchmarks on them.

### [1] Configuration

First, there are 2 files that configure the evaluation process. Both can be found in
**<IoT2/IoT2-Eval-Config>**.

### evaluation-results-config.json

This file configures how to run the automated evaluation. Below are the ones that
probably might need to modify. 

**RESULTS_DIR**: This sets up the result directory organization. You should not need
to change this. The only thing you might want to modify is change <mytool> to the 
name or your tool.

**BOARD**: Specify the board you are using. This is used to lookup the binary from
benchmarks directory.


**ITERATIONS**: The number of times to run each benchmark.

**BENCHMARK_END_BREAKPOINT**: The breakpoint gdb should use to stop the benchmark and
collect the results.

**IOT2_SERIAL_PORT**: The serial port you are using.

#### eval-metric-config.json

This file sets metrics about the board you are using to collect static metrics like
Flash usage. Setup the addresses for these to get accurate evaluation. For 
**EXEC_CODE_REGIONS** put the word/regular experission to idenitify an executable
section. IoT2 uses this to measure the executable code size metric.

#### Other setup files:

If some setup is missing of the above check these files
```
IoT2/IoT2_bench_drivers/iot2_setting.py
IoT2/IoT2/IoT2-lib/IoT2_Config.h
```

### [2] Setup the results directoy

Now to setup the results directory, run the following command:

```
cd IoT2/IoT2_bench_drivers/
python setup_result_collection.py
```

This will create the **results** directory at project root. You will only
need to run this **once**.

### [3] Build the benchmark 

To compile the application. You can run the following command:

```
python iot2_build_benchmarks.py -bc <build_configuration> -b <benchmark_name>
```

***build_configuration***: This is the directory name under 
```BenchIoT/benchmarks``` without the "-benchmarks" suffix.
***benchmark_name***: The name of benchmark directory under 
```BenchIoT/benchmarks/<benchmark_name>``` 

For other options run the script with -h.

### [4] Run the benchmark 

After compiling the application. You can run the following command:

```
# Make sure the network is setup and you do NOT have a serial terminal running
# since this script will launch its own serial terminal
python iot2_run_experiments.py -bc <build_configuration> -b <benchmark_name>
```

The options are the same as before. In order to collect energy note 
that you need to add ```-e```.

To collect the energy measurements, run the following:

```
python compute_power_from_csv.py <energy_csv_results_file>
```

The <energy_csv_results_file> should be in the metric results directory
of the benchmark configuration you used. For example, for mbed-os you should
find the file under <BenchIoT/results/os_results/baseline/metrics_results>


The csv file provides the power measurement during all iterations, to get our
final results (i.e., min, max, and average power) we run the following script
on the resulting csv file
```
# This will print Benchmark, Max, Mean, Min, #Iters to the terminal. The #Iters
# have to match what you used for you configuration (i.e., how  many runs were
# collected). If this number does not match the number of times you configuared
# to run, the an error has occuurred.
./get_all_power.sh  <energy_csv_results_file>
```

## Thanks
This repo includes a version of mbed-os (5.6.5) and its libraries, which use the Apache-2.0 license. Their repo can be found at https://github.com/ARMmbed/mbed-os

It also uses pyterm (https://github.com/RIOT-OS/RIOT/blob/master/dist/tools/pyterm/pyterm), which uses the GNU Lesser General Public License v2.1.

It also uses A selected set of JPEG images from the environmental scene 
database (http://cvcl.mit.edu/database.htm). The details of which can be found in their paper (https://link.springer.com/article/10.1023/A:1011139631724

It also uses libjpeg (http://libjpeg.sourceforge.net/), the details of which is in the directiry itself.

## License
Our modifications and tools are distributed using license in License.md


## Questions? Issues? Errors? Help?

Please raise an issue or contact me through email (nalmakhd and purdue domain) and I will answer you as soon as possible.