
################################################################################
#
# This file is used to include libraries and additional files specific to
# the current benchmark. 
# Libraries are added using the <.mk> files in IoT2_MAKEFILES directory.
#
################################################################################


# The root directory of the project
PROJDIR = $(realpath ../../../../)/

# The benchmark root directory
BENCHDIR := $(CURDIR)/../

# root directory for any extra library
EXT_LIB_ROOT = $(PROJDIR)lib/

# Using the <mbed-lib>.mk file should include the library automatically

# The EVAL_F469NI conflicts with mbed's filesystem lib. So, only link one
# depending on the used board

# include .mk files for EVAL_F469NI
ifeq ($(BOARD), EVAL_F469NI)
include $(PROJDIR)IoT2_MAKEFILES/board-libs-MAKEFILES/EVAL_F469NI_BSP.mk
include $(PROJDIR)IoT2_MAKEFILES/board-libs-MAKEFILES/EVAL_F469NI_DISPLAY.mk
include $(PROJDIR)IoT2_MAKEFILES/board-libs-MAKEFILES/EVAL_F469NI_SDIO.mk
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/IOT2FILESYSTEM_SDIO_INTERFACE.mk

# For other board, use mbed's file system and template display
else
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_SD.mk
include $(PROJDIR)IoT2_MAKEFILES/board-libs-MAKEFILES/IOT2_DISPLAY_TEMPLATE.mk
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/IOT2FILESYSTEM_SPI_INTERFACE.mk
endif

# The below are generic for every board so shoule be linked without issue
include $(PROJDIR)IoT2_MAKEFILES/IOT2LIB.mk
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_OS_IOT2_TCP.mk
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_IOT2_RESULTS_COLLECTOR.mk
include $(PROJDIR)IoT2_MAKEFILES/third-party-MAKEFILES/LIBJPEG.mk


# add -I flag to each inc path from the libraries
INCLUDE_PATHS += $(patsubst %,-I%, $(MBED_LIBS_DIRS))


# add the object files of the libraries. First we convert the src files to 
# their equivalent object file names then add them.
BENCH_LIB_C_OBJS += $(patsubst %.c,./%.o,$(notdir $(MBED_LIBS_C_SRCS)))
BENCH_LIB_CXX_OBJS += $(patsubst %.cpp,./%.o,$(notdir $(MBED_LIBS_CXX_SRCS)))

OBJECTS +=$(BENCH_LIB_C_OBJS)
OBJECTS +=$(BENCH_LIB_CXX_OBJS)

# add the library paths to VPATH so that make will find them
VPATH += $(MBED_LIBS_DIRS)