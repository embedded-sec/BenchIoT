
################################################################################
#
# This is used to automatically include the libjpeg library. If this
# feature is present in the benchmark then this file should be included in the
# APP_EXT_LIBS.mk file. Configuration and driver files are added to the
# benchmark directory itself.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

LIBJPEG_LIB_ROOT = $(PROJDIR)lib/third-party/jpeg-9c/

LIBJPEG_LIB_DIR = $(LIBJPEG_LIB_ROOT)


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
FILTER_OUT_MEMMAC := jmemmac.cpp
FILTER_OUT_MEMDOS := jmemdos.cpp
FILTER_OUT_MEMANSI := jmemansi.cpp
FILTER_OUT_MEMNAME := jmemname.cpp
FILTER_OUT_EXAMPLE := example.cpp
FILTER_OUT_JPEGTRANE := jpegtran.cpp
LIBJPEG_LIB_CXX_SRCS1 = $(filter-out %/$(FILTER_OUT_MEMMAC),\
    $(foreach dir, $(LIBJPEG_LIB_DIR), $(wildcard $(dir)j*.cpp)))
LIBJPEG_LIB_CXX_SRCS2 = $(filter-out %/$(FILTER_OUT_MEMDOS),$(LIBJPEG_LIB_CXX_SRCS1))
LIBJPEG_LIB_CXX_SRCS3 = $(filter-out %/$(FILTER_OUT_MEMANSI),$(LIBJPEG_LIB_CXX_SRCS2))
LIBJPEG_LIB_CXX_SRCS4 = $(filter-out %/$(FILTER_OUT_MEMNAME),$(LIBJPEG_LIB_CXX_SRCS3))
LIBJPEG_LIB_CXX_SRCS5 = $(filter-out %/$(FILTER_OUT_MEMNAME),$(LIBJPEG_LIB_CXX_SRCS4))
LIBJPEG_LIB_CXX_SRCS = $(filter-out %/$(FILTER_OUT_JPEGTRANE),$(LIBJPEG_LIB_CXX_SRCS5))


#LIBJPEG_LIB_CXX_SRCS = $(foreach dir, $(LIBJPEG_LIB_DIR), $(wildcard $(dir)j*.c))


# add lib to mbed lib dirs
MBED_LIBS_DIRS += $(LIBJPEG_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(LIBJPEG_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(LIBJPEG_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).