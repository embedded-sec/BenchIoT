#!/bin/bash

################################################################################
#
# This file is used to setup symbolic links for IoT2 files that are used inside
# mbed. Mainly to initialize IoT2 correctly and automate the configuration
# of runtime measurements.
#
################################################################################


################################## VARIABLS ##################################

CURR_DIR=`dirname \`readlink -f $0\``
MBED_IRQ_DIR=${CURR_DIR}/os-lib/mbed-os/rtos/TARGET_CORTEX/rtx5/TARGET_RTOS_M4_M7/TOOLCHAIN_GCC
IoT2_MBED_DIR=${CURR_DIR}/IoT2/IoT2-mbed-os
IoT2_LIB_DIR=${CURR_DIR}/IoT2/IoT2-lib

SYM_LINK='ln -sfn'

##################################  SETUP   ##################################


# remove files

# remove irq assembly file to add the modified IoT2 one later
rm ${MBED_IRQ_DIR}/irq_cm4f.S

# remove files needed to add IoT2 to boot sequence
rm ${CURR_DIR}/os-lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c
rm ${CURR_DIR}/os-lib/mbed-sdk/platform/mbed_sdk_boot.c
rm ${CURR_DIR}/os-lib/mbed-os/platform/mbed_retarget.cpp


# remove board specific files
# EVAL_F469NI
rm ${CURR_DIR}/os-lib/mbed-os/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_cortex.c

mkdir -p ${CURR_DIR}/lib/IoT2

# Create symlinks

# symlink for build profile with iot2 and debugging information
if ${SYM_LINK} ${IoT2_MBED_DIR}/iot2_debug.json ${CURR_DIR}/os-lib/mbed-os/tools/profiles/iot2_debug.json

then
    echo "[+] Added symlink for iot2_debug.json at: ${CURR_DIR}/os-lib/mbed-os/tools/profiles/iot2_debug.json"
else
    echo "[-] ERROR: Failed to add symlink for iot2_debug.json at: ${CURR_DIR}/os-lib/mbed-os/tools/profiles/iot2_debug.json"
fi



# symlink for build profile with iot2 and debugging information
if ${SYM_LINK} ${IoT2_MBED_DIR}/Makefile.tmpl ${CURR_DIR}/os-lib/mbed-os/tools/\export/makefile/Makefile.tmpl

then
    echo "[+] Added symlink for Makefile.tmpl at: ${CURR_DIR}/os-lib/mbed-os/tools/export/makefile/Makefile.tmpl"
else
    echo "[-] ERROR: Failed to add symlink for Makefile.tmpl at: ${CURR_DIR}/os-lib/mbed-os/tools/export/makefile/Makefile.tmpl"
fi


# symlink for irq*.S
if ${SYM_LINK} ${IoT2_MBED_DIR}/irq_cm4f.S ${MBED_IRQ_DIR}/irq_cm4f.S

then
    echo "[+] Added symlink for ${IoT2_MBED_DIR}/irq_cm4f.S at: ${MBED_IRQ_DIR}/irq_cm4f.S"
else
    echo "[-] ERROR: Failed to add symlink for irq_cm4f.S at: ${MBED_IRQ_DIR}/irq_cm4f.S"
fi


# symlink for mbed-sdk boot sequence
if ${SYM_LINK} ${IoT2_MBED_DIR}/mbed_sdk_boot.c ${CURR_DIR}/os-lib/mbed-os/platform/mbed_sdk_boot.c

then
    echo "[+] Added symlink for mbed_sdk_boot.c at: ${CURR_DIR}/os-lib/mbed-sdk/platform/mbed_sdk_boot.c"
else
    echo "[-] ERROR: Failed to add symlink for mbed_sdk_boot.c at: ${CURR_DIR}/os-lib/mbed-sdk/platform/mbed_sdk_boot.c"
fi

# symlink for mbed-sdk boot sequence
if ${SYM_LINK} ${IoT2_MBED_DIR}/mbed_retarget.cpp ${CURR_DIR}/os-lib/mbed-os/platform/mbed_retarget.cpp

then
    echo "[+] Added symlink for mbed_retarget.cpp at: ${CURR_DIR}/os-lib/mbed-os/platform/mbed_retarget.cpp"
else
    echo "[-] ERROR: Failed to add symlink for mbed_retarget.cpp at: ${CURR_DIR}/os-lib/mbed-os/platform/mbed_retarget.cpp"
fi


# symlink for mbed-os boot sequence
if ${SYM_LINK} ${IoT2_MBED_DIR}/mbed_boot.c ${CURR_DIR}/os-lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c
then
    echo "[+] Added symlink for mbed_boot.c at: ${CURR_DIR}/os-lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c"
else
    echo "[-] ERROR: Failed to add symlink for mbed_boot.c at: ${CURR_DIR}/os-lib/mbed-os/rtos/TARGET_CORTEX/mbed_boot.c"
fi


# symlink for mbed-os TCPSocket to calculate throughput
if ${SYM_LINK} ${IoT2_MBED_DIR}/TCPSocket.cpp ${CURR_DIR}/os-lib/mbed-os/features/netsocket/TCPSocket.cpp
then
    echo "[+] Added symlink for TCPSocket.cpp at: ${CURR_DIR}/os-lib/mbed-os/features/netsocket/TCPSocket.cpp"
else
    echo "[-] ERROR: Failed to add symlink for TCPSocket.cpp at: ${CURR_DIR}/os-lib/mbed-os/features/netsocket/TCPSocket.cpp"
fi


# symlinks for for instrumented serial to measure IO cycels
# add to mbed-os
if ${SYM_LINK} ${IoT2_MBED_DIR}/RawSerial.cpp ${CURR_DIR}/os-lib/mbed-os/drivers/RawSerial.cpp
then
    echo "[+] Added symlink for RawSerial.cpp at: ${CURR_DIR}/os-lib/mbed-os/drivers/RawSerial.cpp"
else
    echo "[-] ERROR: Failed to add symlink for RawSerial.cpp at: ${CURR_DIR}/os-lib/mbed-os/drivers/RawSerial.cpp"
fi

# add symlink to Stream.cpp
if ${SYM_LINK} ${IoT2_MBED_DIR}/Stream.cpp ${CURR_DIR}/os-lib/mbed-os/platform/Stream.cpp
then
    echo "[+] Added symlink for Stream.cpp at: ${CURR_DIR}/os-lib/mbed-os/platform/Stream.cpp"
else
    echo "[-] ERROR: Failed to add symlink for Stream.cpp at: ${CURR_DIR}/os-lib/mbed-os/platform/Stream.cpp"
fi


# symlinks for for instrumented serial to measure IO cycels
# add to mbed-sdk
if ${SYM_LINK} ${IoT2_MBED_DIR}/RawSerial.cpp ${CURR_DIR}/os-lib/mbed-sdk/drivers/RawSerial.cpp
then
    echo "[+] Added symlink for RawSerial.cpp at: ${CURR_DIR}/os-lib/mbed-sdk/drivers/RawSerial.cpp"
else
    echo "[-] ERROR: Failed to add symlink for RawSerial.cpp at: ${CURR_DIR}/os-lib/mbed-sdk/drivers/RawSerial.cpp"
fi

# add symlink to Stream.cpp
if ${SYM_LINK} ${IoT2_MBED_DIR}/Stream.cpp ${CURR_DIR}/os-lib/mbed-sdk/platform/Stream.cpp
then
    echo "[+] Added symlink for Stream.cpp at: ${CURR_DIR}/os-lib/mbed-sdk/platform/Stream.cpp"
else
    echo "[-] ERROR: Failed to add symlink for Stream.cpp at: ${CURR_DIR}/os-lib/mbed-sdk/platform/Stream.cpp"
fi



# symlinks for IoT runtime library
if ${SYM_LINK} ${IoT2_LIB_DIR}/IoT2Lib.c ${CURR_DIR}/lib/IoT2/IoT2Lib.c
then
    echo "[+] Added symlink for IoT2Lib.c at: ${CURR_DIR}/lib/IoT2/IoT2Lib.c"
else
    echo "[-] ERROR: Failed to add symlink for IoT2Lib.c at: ${CURR_DIR}/lib/IoT2/IoT2Lib.c"
fi

if ${SYM_LINK} ${IoT2_LIB_DIR}/IoT2Lib.h ${CURR_DIR}/lib/IoT2/IoT2Lib.h
then
    echo "[+] Added symlink for IoT2Lib.h at: ${CURR_DIR}/lib/IoT2/IoT2Lib.h"
else
    echo "[-] ERROR: Failed to add symlink for IoT2Lib.h at: ${CURR_DIR}/lib/IoT2/IoT2Lib.h"
fi

if ${SYM_LINK} ${IoT2_LIB_DIR}/IoT2_Config.h ${CURR_DIR}/lib/IoT2/IoT2_Config.h
then
    echo "[+] Added symlink for IoT2_Config.h at: ${CURR_DIR}/lib/IoT2/IoT2_Config.h"
else
    echo "[-] ERROR: Failed to add symlink for IoT2_Config.h at: ${CURR_DIR}/lib/IoT2/IoT2_Config.h"
fi

if ${SYM_LINK} ${IoT2_LIB_DIR}/IoT2_Config.c ${CURR_DIR}/lib/IoT2/IoT2_Config.c
then
    echo "[+] Added symlink for IoT2_Config.c at: ${CURR_DIR}/lib/IoT2/IoT2_Config.c"
else
    echo "[-] ERROR: Failed to add symlink for IoT2_Config.c at: ${CURR_DIR}/lib/IoT2/IoT2_Config.c"
fi


# symlink board-specific files
# stmf4 boards
if ${SYM_LINK} ${CURR_DIR}/IoT2/IoT2-mbed-os/board-specific/stm32f4xx_hal_cortex.c ${CURR_DIR}/os-lib/mbed-os/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_cortex.c
then
    echo "[+] Added symlink for stm32f4xx_hal_cortex.c at: ${CURR_DIR}/os-lib/mbed-os/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_cortex.c"
else
    echo "[-] ERROR: Failed to add symlink for stm32f4xx_hal_cortex.c at: ${CURR_DIR}/os-lib/mbed-os/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_cortex.c"
fi

