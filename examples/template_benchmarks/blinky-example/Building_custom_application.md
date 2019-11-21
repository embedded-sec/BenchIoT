

## Setup

In order to use this example, make sure you already ran the following script
from the project root to setup a symlink for the main file and mbed-os.

```
./setup.sh
```

Also, we assume you have OpenOCD up and running for your board.

## Building and running the application

**Note that (IoT2/iot2) is used here as a shortcut for BenchIoT files, script, and functions.**

See the list of options below and use the one that works for you. I will be using EVAL_F469NI as the board for the examples below. The same command should run when you change 
that to your board. (e.g., DISCO_F407VG, K64F...etc)


This creats a makefile in the application directory, you can change iot2_debug
to release if you want to build with -Os

```
mbed export -m EVAL_F469NI -i GCC_ARM --profile iot2_debug
```

Now build the benchmark

```
# A binary should be built in both the (bins & BUILD) directories. Note
# that in case you would like to build with an OS then REMOVE IOT2_BM=1
# from the command below
make -j4 BOARD=EVAL_F469NI IOT2_ALL=1 IOT_BM=1 CUSTOM_BIN_SUFFIX=--baseline iot2benchmark
```
Now you can run the benchmark using gdb. Note that gdb_helpers.py is a script
to be used by the gdb-python to ease the debugging process.
```
# The support_scripts directory is at the root of the project directory.
<YOUR_GDB> <BINARY_FILE> -x support_scripts/gdb_helpers.py
```

This simple example is using the serial, you might need to run pyterm to see 
the output throught the terminal. You should find a copy in support_scripts

```
sudo python pyterm.py -p <YOUR_PORT> -b <BAUD_RATE>
```

Below is a brief explanation of the above:

- CUSTOM_BIN_SUFFIX: This just adds your customized suffix to the name of binary.
- IOT2_ALL: Sets the macros to enable the iot2 runtime library.
- iot2benchmark: Compile and links IoT2Lib with application. You can use <all> 
instead if you do not use any of the iot2 runtime library features. Note that 
you will need to remove all iot2 calls in the app.
- YOUR_GDB: Your arm-none-eabi-gdb.
- BINARY_FILE: This the <.elf> file you want to run.
- YOUR_PORT: After you connect your board do a `ls /dev/tty*`, and see if there 
is a new port for the board. Usually this is either /dev/ttyUSB0 or /dev/ttyACM0
- BAUD_RATE: This is the speed of communication, usually 9600. Check the 
application to verify.



After running the application, it will blink an LED and print a message through
the serial terminal, at the end it will display the metrics in the terminal.

```
INFO # [IoT2] benchmark: START
INFO #  Welcome to IoT2
INFO # Blinky-example application Running!
INFO # Blinky-example application Running!
INFO # Blinky-example application Running!
INFO # Blinky-example application Running!
INFO # Blinky-example application Running!
INFO # Blinky-example application Running!
.
.
.
.
INFO # [IoT2] collect_results: START
INFO # IoT2->TotalRuntime_cycles:.....
INFO # IoT2->ISR_cycles:....
INFO # IoT2->SVC_cycles:....
INFO # IoT2->PendSV_cycles:....
INFO # IoT2->SysTick_cycles:....
INFO # IoT2->Init_cycles:....
INFO # IoT2->IoT2Overhead_cycles:....
INFO # IoT2->PrivThread_cycles:....
INFO # IoT2->Sleep_cycles:....
INFO # IoT2->TotalPriv_code:....
INFO # IoT2->DEP_Enabled:....
INFO # IoT2->TotalPriv_cycles:....
INFO # IoT2->Exception_cycles:....
INFO # IoT2->EXCCNT_cycles:....
INFO # [IoT2] collect_results: END

```

Note that to enable automatic collection of the metrics, you will need to copy
the application directory to the benchmark directory (either in mbed-benchmarks
for baremetal or benchmarks/mbed-os-benchmarks). Note that you will need to
close the serial terminal as the evaluation framework uses its own to collect
the metrics through the serial. The instruction for automatically running the
benchmarks can be found in the README.
