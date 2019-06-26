
################################################################################
#
# This is used to automatically include the micro-sd feature of mbed. If this
# feature is present in the benchmark then this file should be included in the
# mbed-os/mbed-sdk makefile.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

SDIO_LIB_ROOT = $(PROJDIR)lib/board-specific/sdio/

SDIO_LIB_DIR = $(SDIO_LIB_ROOT)STM/SDIO_EVAL_F469NI
SDIO_LIB_DIR += $(SDIO_LIB_ROOT)STM/SDIO_EVAL_F469NI/option
#SDIO_LIB_DIR += $(MBED_LIBS_ROOT)STM/SDIO_EVAL_F469NI/drivers


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
SDIO_LIB_C_SRCS = $(foreach dir,$(SDIO_LIB_DIR),$(wildcard $(dir)/ff.c))
SDIO_LIB_C_SRCS += $(foreach dir,$(SDIO_LIB_DIR),$(wildcard $(dir)/diskio.c))
SDIO_LIB_C_SRCS += $(foreach dir,$(SDIO_LIB_DIR),$(wildcard $(dir)/ff_gen_drv.c))
SDIO_LIB_C_SRCS += $(foreach dir,$(SDIO_LIB_DIR),$(wildcard $(dir)/sd_diskio.c))
SDIO_LIB_C_SRCS += $(foreach dir,$(SDIO_LIB_DIR),$(wildcard $(dir)/syscall.c))
SDIO_LIB_CXX_SRCS = $(foreach dir,$(SDIO_LIB_DIR),$(wildcard $(dir)/IoT2SDIOFatFSInterface.cpp))


# add sd lib to mbed lib dirs
MBED_LIBS_DIRS += $(SDIO_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(SDIO_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(SDIO_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).