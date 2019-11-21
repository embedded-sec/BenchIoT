#!/bin/bash

################################################################################
#
# This file is used to setup symbolic links for external libraries so that
# they are added in a unified method wether we are using mbed-os or mbed-sdk.
#
################################################################################


################################## VARIABLS ##################################

CURR_DIR=`dirname \`readlink -f $0\``
MBED_OS='os-lib/mbed-os'
MBED_FEATURES='os-lib/mbed-os/features'
BENCHMARKS='benchmarks'
OS_BENCHMARKS='benchmarks/mbed-os-benchmarks'
EXAMPLES='examples'
MBED_OS_DIR=${CURR_DIR}/${MBED_OS}
FEATURES_DIR=${CURR_DIR}/${MBED_FEATURES}
BENCHMARKS_DIR=${CURR_DIR}/${BENCHMARKS}
OS_BENCHMARKS_DIR=${CURR_DIR}/${OS_BENCHMARKS}
EXAMPLES_DIR=${CURR_DIR}/${EXAMPLES}
MBED_LIB_DIR='lib/mbed-libs/FEATURES'
LIB_DIR=${CURR_DIR}/${MBED_LIBS}

# Path to lwip for the BAREMETAL version
BAREMETAL_LWIPSRC_PATH='lib/mbed-libs/BAREMETAL_LWIP/lwip'
# The symlink we would like to add
BAREMETAL_LWIPSRC_DIR=${CURR_DIR}/${BAREMETAL_LWIPSRC_PATH}

# Actual path to LWIP in the FEATURE_LWIP directory
FEATURE_LWIP_LWIPSRC_PATH='os-lib/mbed-os/features/FEATURE_LWIP/lwip-interface/lwip' 
FEATURE_LWIP_LWIPSRC_DIR=${CURR_DIR}/${FEATURE_LWIP_LWIPSRC_PATH}

SYM_LINK='ln -sTf'

##################################  SETUP   ##################################

 
echo "========================= Welcome to IoT2 setup ========================="

# Create Features directory in mbed-libs
mkdir -p ${MBED_LIB_DIR}

echo "===--------------------- Creating Symlinks --------------------==="


# Loop through the fetures directory and create a symlink in the lib directory
for feature in ${FEATURES_DIR}/*; do

	if [ -d "$feature" ]; then
		feature_basename=`basename ${feature}`
		${SYM_LINK} ${feature}/ ${MBED_LIB_DIR}/${feature_basename}
		echo "[+] Added symlink for:"${feature_basename}
	fi
done

# Now add the symlink for BARMETAL_LWIP
if ${SYM_LINK} ${FEATURE_LWIP_LWIPSRC_DIR}/ ${BAREMETAL_LWIPSRC_DIR}
then
	echo "[+] Added symlink for: lwip in ($BAREMETAL_LWIPSRC_DIR)"
else
	echo "[-] ERROR: Failed to add symlink for lwip in ($BAREMETAL_LWIPSRC_DIR)"
fi


echo "------------------------------------------------------------------"
echo "Setting mbed-os symlink for benchmarks"
echo "------------------------------------------------------------------"

# setting symlinks for benchmarks directory
for benchmark_type in ${BENCHMARKS_DIR}/*; do

    if [ -d "$benchmark_type" ]; then
        echo "[+] name: "${benchmark_type}

        for benchmark in ${benchmark_type}/*; do
            if [ -d "$benchmark" ]; then
                if ${SYM_LINK} ${MBED_OS_DIR}/ ${benchmark}/mbed-os
                then
                    echo "          [+] Added mbed-os symlink in:"${benchmark}
                else
                    echo "          [-] ERROR: Failed to add mbed-os symlink in:"${benchmark}
                fi
            fi
        done
    fi
done


# setting symlinks for example directory
for benchmark_type in ${EXAMPLES_DIR}/*; do

    if [ -d "$benchmark_type" ]; then
        echo "[+] name: "${benchmark_type}

        for benchmark in ${benchmark_type}/*; do
            if [ -d "$benchmark" ]; then
                if ${SYM_LINK} ${MBED_OS_DIR}/ ${benchmark}/mbed-os
                then
                    echo "          [+] Added mbed-os symlink in:"${benchmark}
                else
                    echo "          [-] ERROR: Failed to add mbed-os symlink in:"${benchmark}
                fi
            fi
        done
    fi
done



echo "===---------------------  Symlinks: DONE  ---------------------==="

echo "===--------------------- Setting up Tools ---------------------==="

#./setup_tools.sh
./setup_IoT2.sh
./setup_main_files.sh

echo "===----------------------- Tools: DONE ------------------------==="

echo "=========================== IoT2 setup: DONE ============================"