
################################################################################
#
# This file is used to automatically include mbed-http(s) feature. If this
# feature is present in the benchmark then this file should be included in the
# Benchmark.mk file of the application.
#
################################################################################



# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

# Target specific files have not been added. If needed modify this file
# to include the folders using ifeg ($(BOARD), <your_target>)
MBEDHTTP_LIB_DIR = $(PROJDIR)lib/mbed-http
MBEDHTTP_LIB_DIR += $(PROJDIR)lib/mbed-http/source
MBEDHTTP_LIB_DIR += $(PROJDIR)lib/mbed-http/http_parser

# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
MBEDHTTP_LIB_C_SRCS = $(foreach dir,$(MBEDHTTP_LIB_DIR),$(wildcard $(dir)/*.c))
MBEDHTTP_LIB_CXX_SRCS = $(foreach dir,$(MBEDHTTP_LIB_DIR),$(wildcard $(dir)/*.cpp))


# add mbedtls lib to mbed lib dirs
MBED_LIBS_DIRS += $(MBEDHTTP_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(MBEDHTTP_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(MBEDHTTP_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).




