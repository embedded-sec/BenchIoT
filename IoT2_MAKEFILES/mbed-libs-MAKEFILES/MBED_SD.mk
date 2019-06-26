
################################################################################
#
# This is used to automatically include the micro-sd feature of mbed. If this
# feature is present in the benchmark then this file should be included in the
# mbed-os/mbed-sdk makefile.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk


SD_LIB_DIR = $(MBED_LIBS_ROOT)sd


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
SD_LIB_C_SRCS = $(foreach dir,$(SD_LIB_DIR),$(wildcard $(dir)/*.c))
SD_LIB_CXX_SRCS = $(foreach dir,$(SD_LIB_DIR),$(wildcard $(dir)/*.cpp))


# add sd lib to mbed lib dirs
MBED_LIBS_DIRS += $(SD_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(SD_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(SD_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).