

################################################################################
#
# This is used to setup the variables that will be used by other *.mk files in
# this directory. The makefile building mbed-os/mbed-sdk will include each
# libraries *.mk file, and all of these will be added to the MBED_LIBS_*
# variables which eventually are added to the mbed-os/mbed-sdk makefile.
# This design allows a cleaner way of adding each library by just adding
# the respective *.mk file without needing to modify the mbed-os/mbed-sdk
# makefile every time for each application.
#
################################################################################


# Include board configuration using the given benchmark path
include $(BENCHDIR)/Benchmark.mk
include $(PROJDIR)IoT2_MAKEFILES/mbed-os-MAKEFILES/Board.mk
include $(BENCHDIR)/APP_EXT_LIBS.mk
include $(PROJDIR)IoT2_MAKEFILES/mbed-os-MAKEFILES/MBED_OS_COMMON_FLAGS.mk
include $(PROJDIR)IoT2_MAKEFILES/TOOLCHAIN.mk

#==========================   mbed paths  	  ==========================#

MBEDBASE:=$(PROJDIR)os-lib/mbed-os
BUILDDIR =$(BENCHDIR)/build/$(BOARD)/
DEPDIR = $(BUILDDIR)dep/
MBED_OBJS = $(BUILDDIR)mbed_objs/

# mbed-os dirs
EVENTS := events
EQUEUE := equeue
RTOS := rtos
RTX5 := rtx5
RTX4 := rtx4
TARGET_RTOS_M4_M7 := TARGET_RTOS_M4_M7
FEATURES := features

#INC paths
MBED_DIRS = $(MBEDBASE)
MBED_DIRS += $(MBEDBASE)/$(CMSIS)
MBED_DIRS += $(MBEDBASE)/$(CMSIS)/$(TARGET_PREFIX)$(TARGET_ARCH)
MBED_DIRS += $(MBEDBASE)/$(CMSIS)/$(TARGET_PREFIX)$(TARGET_ARCH)/$(TOOLCHAIN_PREFIX)
MBED_DIRS += $(MBEDBASE)/$(DRIVERS)
MBED_DIRS += $(MBEDBASE)/$(HAL)
MBED_DIRS += $(MBEDBASE)/$(HAL)/storage_abstraction
MBED_DIRS += $(MBEDBASE)/$(PLATFORM)
MBED_DIRS += $(MBEDBASE)/$(EVENTS)
MBED_DIRS += $(MBEDBASE)/$(EVENTS)/$(EQUEUE)
MBED_DIRS += $(MBEDBASE)/$(RTOS)
MBED_DIRS += $(MBEDBASE)/$(RTOS)/$(TARGET_CORTEX)
MBED_DIRS += $(MBEDBASE)/$(RTOS)/$(TARGET_CORTEX)/$(RTX4)
MBED_DIRS += $(MBEDBASE)/$(RTOS)/$(TARGET_CORTEX)/$(RTX5)
MBED_DIRS += $(MBEDBASE)/$(RTOS)/$(TARGET_CORTEX)/$(RTX5)/$(TARGET_RTOS_M4_M7)/$(TOOLCHAIN_PREFIX)
MBED_DIRS += $(MBED_LIBS_FEATURES_ROOT)


# add board specific paths as suffix
MBED_DIRS += $(patsubst %, $(MBEDBASE)/$(TARGETS)/$(TARGET_PREFIX)%, $(BOARD_PATHS))

# add -I flag to each inc path
MBED_PATHS += $(patsubst %, -I%, $(MBED_DIRS))

# Add the MBED_LIBS_PATHS to MBED_PATHS
MBED_LIBS_PATHS = $(patsubst %, -I%, $(MBED_LIBS_DIRS))
MBED_PATHS += $(MBED_LIBS_PATHS)

# generate list of source files for mbed-os (for c,cpp and assembly files).
# Make sure to exclude mbed-sdk bootup files later.
C_SRCS_WITH_SDK_BOOT_SRCS = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*.c))
CXX_SRCS = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*.cpp))
ASM_SRCS_MBED_AND_IoT2 = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*.S))



#-------------------------------------------------------------------------------
# Fixes to enable choosing linker script

ALL_LINKER_SCRIPTS = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*.ld))

ifeq ($(BOOTLOADER), 1)
LINKER_SCRIPT = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*bootloader.ld))
else ifeq ($(WATCHDOG), 1)
LINKER_SCRIPT=$(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*experiment.ld))
else ifeq ($(SECURE_DATA), 1)
LINKER_SCRIPT=$(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*secure_data.ld))
else
BOOTLOADER_LD = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*bootloader.ld))
EXPERIMENT_LD = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*experiment.ld))
SECURE_DATA_LD = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/*experiment.ld))
# remove bootloader
NO_BL_LINKER_SCRIPTS = $(filter-out $(BOOTLOADER_LD), $(ALL_LINKER_SCRIPTS))
# remove experiment linker script
NO_EXPER_LINKER_SCRIPT = $(filter-out $(EXPERIMENT_LD), $(NO_BL_LINKER_SCRIPTS))
#filter out secure data
LINKER_SCRIPT = $(filter-out $(SECURE_DATA_LD), $(NO_EXPER_LINKER_SCRIPT))

endif

#-------------------------------------------------------------------------------



#----------------------- REMOVE MBED-SDK BOOTING FILES ------------------------#

# Exclude files specific to mbed-sdk bootup. Mbed-os uses different files,
# namely mbed_boot.c and mbed_wait_api_rtos.c.

MBED_SDK_BOOTUP_SCRS = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/mbed_wait_api_no_rtos.c))
MBED_SDK_BOOTUP_SCRS += $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/mbed_sdk_boot.c))

C_SRCS = $(filter-out $(MBED_SDK_BOOTUP_SCRS), $(C_SRCS_WITH_SDK_BOOT_SRCS))

#------------------------------------------------------------------------------#


#----------------------- IoT2 measurement Configuration -----------------------#

MBED_ASM_SRCS = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/irq_cm*.S))
IoT2_ASM_SRCS = $(foreach dir, $(MBED_DIRS), $(wildcard $(dir)/IoT2_irq_cm*.S))

# Measure runtime metrics, thus include IoT2 modified file
ifeq ($(IOT2), 1)

# Include IOT2 or not
CC_DIRECTIVES += -DIOT2
CXX_DIRECTIVES += -DIOT2

# if traking ISR runtime measurements
ifeq ($(IOT2_ISR_RUNTIME), 1)
CC_DIRECTIVES += -DIOT2_ISR_RUNTIME
CXX_DIRECTIVES += -DIOT2_ISR_RUNTIME
ASM_SRCS = $(filter-out $(MBED_ASM_SRCS), $(ASM_SRCS_MBED_AND_IoT2))

# Collect iot2 metrics and send it to remote server
ifeq ($(IOT2_RESULTS_COLLECTOR), 1)
# Include result collection
CC_DIRECTIVES += -DIOT2_RESULTS_COLLECTOR
CXX_DIRECTIVES += -DIOT2_RESULTS_COLLECTOR
endif


# Do not measure runtime metrics, thus do not use IoT2
else
ASM_SRCS = $(filter-out $(IoT2_ASM_SRCS), $(ASM_SRCS_MBED_AND_IoT2))

endif

# Do not measure runtime metrics, thus do not use IoT2
else
ASM_SRCS = $(filter-out $(IoT2_ASM_SRCS), $(ASM_SRCS_MBED_AND_IoT2))

endif

#------------------------------------------------------------------------------#

# Add the MBED_LIBS src files
C_SRCS += $(MBED_LIBS_C_SRCS)
CXX_SRCS += $(MBED_LIBS_CXX_SRCS)


# generate corresponding *.o files for the above
C_OBJS := $(patsubst %.c, $(MBED_OBJS)%.o, $(notdir $(C_SRCS)))
CXX_OBJS := $(patsubst %.cpp, $(MBED_OBJS)%.o, $(notdir $(CXX_SRCS)))
ASM_OBJS := $(patsubst %.S, $(MBED_OBJS)%.o, $(notdir $(ASM_SRCS)))

# setup VPATH
VPATH += $(MBED_DIRS)
VPATH += $(MBED_LIBS_DIRS)
