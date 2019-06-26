
################################################################################
#
# This file is used to automatically include iot2fs interface. If this
# feature is present in the benchmark then this file should be included in the
# Benchmark.mk file of the application.
#
################################################################################



# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

# Folder of iot2TCP
IOT2FS_SPI_INTERFACE_DIR = $(PROJDIR)/IoT2/IoT2-mbed-wrappers/IoT2_wrappers/spi


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
IOT2FS_SPI_INTERFACE_C_SRCS = $(foreach dir,$(IOT2FS_SPI_INTERFACE_DIR),$(wildcard $(dir)/*.c))
IOT2FS_SPI_INTERFACE_CXX_SRCS = $(foreach dir,$(IOT2FS_SPI_INTERFACE_DIR),$(wildcard $(dir)/*.cpp))


# add result_collection lib to mbed lib dirs
MBED_LIBS_DIRS += $(IOT2FS_SPI_INTERFACE_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(IOT2FS_SPI_INTERFACE_C_SRCS)
MBED_LIBS_CXX_SRCS += $(IOT2FS_SPI_INTERFACE_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).




