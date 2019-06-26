
################################################################################
#
# This is used to automatically include the LWIP feature of mbed-sdk. If this
# feature is presesnt in the benchmark then this file should be included in the
# APP_EXT_LIBS.mk file.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk


# LWIP lib dir
BAREMETAL_LWIP_DIR = $(MBED_LIBS_ROOT)BAREMETAL_LWIP

#----------------------------- BOARD specific files ---------------------------#

### K64F-LWIP ###
ifeq ($(BOARD), K64F)
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip-eth/arch/TARGET_Freescale
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip-eth/arch/TARGET_Freescale/TARGET_K64F

### EVAL_F469NI ###
else ifeq ($(BOARD), EVAL_F469NI)
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip-eth/arch/TARGET_STM/TARGET_STM32F4/TARGET_EVAL_F469NI
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip-eth/arch/TARGET_STM


# if board not found, through an error
else
$(error [ERROR]: BAREMETAL_LWIP Configurations for <$(BOARD)> has not been added.\
 Please add it in  BAREMETAL_LWIP.mk file)
endif

#------------------------------ Lib generic files -----------------------------#

# generic LWIP files
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip-sys
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip-sys/arch
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src
#BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/api
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/core
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/core/ipv4
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/core/ipv6
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/netif
#BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/netif/ppp
#BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/netif/ppp/polarssl
# The next directories are only for include paths (i.e., header files)
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include/lwip
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include/lwip/priv
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include/lwip/prot
BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include/netif
#BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include/netif/ppp
#BAREMETAL_LWIP_DIR += $(MBED_LIBS_ROOT)BAREMETAL_LWIP/lwip/src/include/netif/ppp/polarssl



# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
BAREMETAL_LWIP_LIB_C_SRCS = $(foreach dir,$(BAREMETAL_LWIP_DIR),$(wildcard $(dir)/*.c))
BAREMETAL_LWIP_LIB_CXX_SRCS = $(foreach dir,$(BAREMETAL_LWIP_DIR),$(wildcard $(dir)/*.cpp))

# Add the mbed LWIP directory to mbed libs dir
MBED_LIBS_DIRS += $(BAREMETAL_LWIP_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(BAREMETAL_LWIP_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(BAREMETAL_LWIP_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).