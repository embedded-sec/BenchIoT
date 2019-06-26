
################################################################################
#
# This is used to automatically include the display werapper class of a specific
# board. Note that other board specific files are added in the APP_EXT_LIBS.mk
# file. If such files are added here make sure they are removed from the 
# APP_EXT_LIBS.mk file.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

DISPLAY_LIB_ROOT = $(PROJDIR)lib/board-specific/display/

DISPLAY_LIB_DIR = $(DISPLAY_LIB_ROOT)ST
#DISPLAY_LIB_DIR += $(MBED_LIBS_ROOT)STM/DISPLAY_EVAL_F469NI/drivers


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
DISPLAY_LIB_C_SRCS = $(foreach dir,$(DISPLAY_LIB_DIR),$(wildcard $(dir)/*.c))

DISPLAY_LIB_CXX_SRCS = $(foreach dir,$(DISPLAY_LIB_DIR),$(wildcard $(dir)/*.cpp))


# add sd lib to mbed lib dirs
MBED_LIBS_DIRS += $(DISPLAY_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(DISPLAY_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(DISPLAY_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).