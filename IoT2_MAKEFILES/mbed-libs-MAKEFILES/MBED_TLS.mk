
################################################################################
#
# This file is used to automatically include mbed-tls feature. If this
# feature is present in the benchmark then this file should be included in the
# APP_EXT_LIBS.mk file of the application.
#
################################################################################



# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

# Target specific files have not been added. If needed modify this file
# to include the folders using ifeg ($(BOARD), <your_target>)
MBEDTLS_LIB_DIR = $(MBED_LIBS_FEATURES_ROOT)mbedtls
MBEDTLS_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)mbedtls/inc
MBEDTLS_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)mbedtls/platform
MBEDTLS_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)mbedtls/platform/inc
MBEDTLS_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)mbedtls/platform/src
MBEDTLS_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)mbedtls/src


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
MBEDTLS_LIB_C_SRCS = $(foreach dir,$(MBEDTLS_LIB_DIR),$(wildcard $(dir)/*.c))
MBEDTLS_LIB_CXX_SRCS = $(foreach dir,$(MBEDTLS_LIB_DIR),$(wildcard $(dir)/*.cpp))


# add mbedtls lib to mbed lib dirs
MBED_LIBS_DIRS += $(MBEDTLS_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(MBEDTLS_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(MBEDTLS_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).




