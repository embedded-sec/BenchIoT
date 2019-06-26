#!/bin/bash

################################################################################
#
# This file is used to setup symbolic links for main files that are used with 
# the benchmark depending on the configuration of the benchmark. Check the 
# README.md file in main_files directory for more details
#
################################################################################


################################## VARIABLS ##################################

CURR_DIR=`dirname \`readlink -f $0\``
MBED_OS='os-lib/mbed-os'
MBED_FEATURES='os-lib/mbed-os/features'
BENCHMARKS='benchmarks'
OS_BENCHMARKS='benchmarks/mbed-os-benchmarks'
SECURE_DATA_OS_BENCHMARKS='benchmarks/secure_data_OS-benchmarks'
SECURE_DATA_BM_BENCHMARKS='benchmarks/secure_data_SDK-benchmarks'
MBED_OS_DIR=${CURR_DIR}/${MBED_OS}
FEATURES_DIR=${CURR_DIR}/${MBED_FEATURES}
BENCHMARKS_DIR=${CURR_DIR}/${BENCHMARKS}
OS_BENCHMARKS_DIR=${CURR_DIR}/${OS_BENCHMARKS}
SECURE_DATA_OS_BENCHMARKS_DIR=${CURR_DIR}/${SECURE_DATA_OS_BENCHMARKS}
SECURE_DATA_BM_BENCHMARKS_DIR=${CURR_DIR}/${SECURE_DATA_BM_BENCHMARKS}
MBED_LIB_DIR='lib/mbed-libs/FEATURES'
LIB_DIR=${CURR_DIR}/${MBED_LIBS}

# Path to lwip for the BAREMETAL version
BAREMETAL_LWIPSRC_PATH='lib/mbed-libs/BAREMETAL_LWIP/lwip'
# The symlink we would like to add
BAREMETAL_LWIPSRC_DIR=${CURR_DIR}/${BAREMETAL_LWIPSRC_PATH}

# Actual path to LWIP in the FEATURE_LWIP directory
FEATURE_LWIP_LWIPSRC_PATH='os-lib/mbed-os/features/FEATURE_LWIP/lwip-interface/lwip' 
FEATURE_LWIP_LWIPSRC_DIR=${CURR_DIR}/${FEATURE_LWIP_LWIPSRC_PATH}

# path to main file for secure_data benchmarks
SECURE_DATA_MAIN_FILE_PATH='main_files/securedata_main.cpp'
SECURE_DATA_MAIN_FILE=${CURR_DIR}/${SECURE_DATA_MAIN_FILE_PATH}
# path to main file for other benchmarks
GENERAL_MAIN_FILE_PATH='main_files/main.cpp'
GENERAL_MAIN_FILE=${CURR_DIR}/${GENERAL_MAIN_FILE_PATH}

SYM_LINK='ln -sTf'

##################################  SETUP   ##################################

 
echo "-------------------------------------------------------------------------"
echo "[*] Setting symlinks to main files for each benchmark"


# Loop through the epoxy benchmarks and create mbed-os symlinks
for benchmark_type in ${BENCHMARKS_DIR}/*; do

    if [ -d "$benchmark_type" ]; then
        echo "[+] name: "${benchmark_type}
        #-----------------------------------------------------------------------
        # secure data (OS) benchmarks
        #-----------------------------------------------------------------------
        if [ "$benchmark_type" == "$SECURE_DATA_OS_BENCHMARKS_DIR" ]; then
            for benchmark in ${benchmark_type}/*; do
                if [ -d "$benchmark" ]; then
                    # remove main.cpp and symlink for watchdog_main.cpp
                    if rm ${benchmark}/main.cpp 
                    then
                        echo "      [+] removed main.cpp from ${benchmark_type}/${benchmark}"
                    else
                        echo "      [+] No file named main.cpp found in ${benchmark_type}/${benchmark}"
                    fi
                    # add symlink
                    if ${SYM_LINK} ${SECURE_DATA_MAIN_FILE} ${benchmark}/securedata_main.cpp
                    then
                        echo "      [+] Added symlink for uvisor_main.cpp at: ${benchmark}/uvisor_main.cpp"
                    else
                        echo "      [-] ERROR: Failed to add symlink for uvisor_main.cpp at: ${benchmark}/securedata_main.cpp"
                    fi
                fi
            done

        #-----------------------------------------------------------------------
        # secure data (BM) benchmarks
        #-----------------------------------------------------------------------
        elif [ "$benchmark_type" == "$SECURE_DATA_BM_BENCHMARKS_DIR" ]; then
            for benchmark in ${benchmark_type}/*; do
                if [ -d "$benchmark" ]; then
                    # remove main.cpp and symlink for watchdog_main.cpp
                    if rm ${benchmark}/main.cpp 
                    then
                        echo "      [+] removed main.cpp from ${benchmark_type}/${benchmark}"
                    else
                        echo "      [+] No file named main.cpp found in ${benchmark_type}/${benchmark}"
                    fi
                    # add symlink
                    if ${SYM_LINK} ${SECURE_DATA_MAIN_FILE} ${benchmark}/securedata_main.cpp
                    then
                        echo "      [+] Added symlink for uvisor_main.cpp at: ${benchmark}/uvisor_main.cpp"
                    else
                        echo "      [-] ERROR: Failed to add symlink for uvisor_main.cpp at: ${benchmark}/securedata_main.cpp"
                    fi
                fi
            done
        #-----------------------------------------------------------------------
        # Other benchmarks
        #-----------------------------------------------------------------------
        else
            for benchmark in ${benchmark_type}/*; do
                if [ -d "$benchmark" ]; then
                    # remove main.cpp and symlink for watchdog_main.cpp
                    if rm ${benchmark}/main.cpp 
                    then
                        echo "      [+] removed main.cpp from ${benchmark_type}/${benchmark}"
                    else
                        echo "      [+] No file named main.cpp found in ${benchmark_type}/${benchmark}"
                    fi
                    # add symlink
                    if ${SYM_LINK} ${GENERAL_MAIN_FILE} ${benchmark}/main.cpp
                    then
                        echo "      [+] Added symlink for main.cpp at: ${benchmark}/main.cpp"
                    else
                        echo "      [-] ERROR: Failed to add symlink for main.cpp at: ${benchmark}/main.cpp"
                    fi
                fi
            done
        fi

    fi
done

echo "-------------------------------------------------------------------------"