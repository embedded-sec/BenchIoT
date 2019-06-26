
################################################################################
#
# This file is used to automatically include IoT2 library. If this
# feature is present in the benchmark then this file should be included in the
# Benchmark.mk file of the application.
#
################################################################################



# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

IOT2LIB_DIR = $(PROJDIR)lib/IoT2

IOT2LIB_OBJECTS_DIR = ./IoT2

##################################
# If IoT2_ALL=1
##################################

# This is a short hand to enable all the IoT2Lib flags.
ifeq ($(IOT2_ALL), 1)
IOT2 := 1
IOT2_ISR_RUNTIME := 1
IOT2_RESULTS_COLLECTOR := 1
endif

##################################
# If IoT2=1
##################################

# Enable IoT2 runtime library but do not intercept exception. All other runtime
# metrics are enabled.
ifeq ($(IOT2), 1)

# add the IoT2 directive
C_FLAGS += -DIOT2
CXX_FLAGS += -DIOT2

# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
IOT2LIB_C_SRCS = $(foreach dir,$(IOT2LIB_DIR),$(wildcard $(dir)/*.c))

IOT2LIB_OBJECTS += $(patsubst %.c,$(IOT2LIB_OBJECTS_DIR)/%.o,$(notdir $(IOT2LIB_C_SRCS)))

# add -I flag to each inc path from the libraries
INCLUDE_PATHS += $(patsubst %,-I%, $(IOT2LIB_DIR))

##################################
# If IOT2_ISR_RUNTIME=1
##################################

# Enables the runtime library to intercept and measure exception cycels (e.g.,
# SVC, interrupts...etc)
ifeq ($(IOT2_ISR_RUNTIME), 1)

# enable tracking the vector table
# add the IoT2 directive
C_FLAGS += -DIOT2_ISR_RUNTIME
CXX_FLAGS += -DIOT2_ISR_RUNTIME
ASM_FLAGS += -DIOT2_ISR_RUNTIME
endif


##################################
# If IOT2_RESULTS_COLLECTOR=1
##################################

# Enables reporting the runtime metrics at the end of the benchmark execution.
ifeq ($(IOT2_RESULTS_COLLECTOR), 1)
C_FLAGS += -DIOT2_RESULTS_COLLECTOR
CXX_FLAGS += -DIOT2_RESULTS_COLLECTOR
endif 



# finally, make sure IoT2lib is always included
C_FLAGS += -include
C_FLAGS += IoT2Lib.h

CXX_FLAGS += -include
CXX_FLAGS += IoT2Lib.h

ASM_FLAGS += -I$(IOT2LIB_OBJECTS_DIR)


# add iot2 to VPATH
VPATH += $(IOT2LIB_DIR)


##################################
# This defines the setup used to IoT2_Config.c. Check the begininng of
# IoT2_Config.h for details. If nothing is specified then the 
# default configuration (IOT2_DEFAULT_CONFIG) is used
# If IOT2_CONFIG=
# 				  0 --> IOT2_DEFAULT_CONFIG
# 				  1 --> SECURE_WATCHDOG_CONFIG
# 				  2 --> SECURE_DATA_CONFIG
# 				  3 --> IOT2_UVISOR_CONFIG
# 
##################################
ifeq ($(IOT2_CONFIG), 1)
C_FLAGS += -DIOT2_CONFIGUATION=1
CXX_FLAGS += -DIOT2_CONFIGUATION=1
else ifeq ($(IOT2_CONFIG), 2)
C_FLAGS += -DIOT2_CONFIGUATION=2
CXX_FLAGS += -DIOT2_CONFIGUATION=2
else ifeq ($(IOT2_CONFIG), 3)
C_FLAGS += -DIOT2_CONFIGUATION=3
CXX_FLAGS += -DIOT2_CONFIGUATION=3
else
C_FLAGS += -DIOT2_CONFIGUATION=0
CXX_FLAGS += -DIOT2_CONFIGUATION=0
endif

# if baremetal
ifeq ($(IOT2_BM), 1)
C_FLAGS += -DIoT2_OS_BENCHMARKS=0
CXX_FLAGS += -DIoT2_OS_BENCHMARKS=0
ASM_FLAGS += -DIoT2_OS_BENCHMARKS=0
else
C_FLAGS += -DIoT2_OS_BENCHMARKS=1
CXX_FLAGS += -DIoT2_OS_BENCHMARKS=1
ASM_FLAGS += -DIoT2_OS_BENCHMARKS=1
endif

##################################
# If IOT2_UVISOR=1
##################################
ifeq ($(IOT2_UVISOR), 1)

C_FLAGS += -DIOT2_UVISOR
CXX_FLAGS += -DIOT2_UVISOR
ASM_FLAGS += -DIOT2_UVISOR
# add the missing flags from mbed-cli export when using uvisor
ASM_FLAGS += -DFEATURE_UVISOR=1
ASM_FLAGS += -DTARGET_UVISOR_SUPPORTED
endif

endif

# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).




