
################################################################################
#
# This is used to automatically include the LWIP feature of mbed. If this
# feature is presesnt in the benchmark then this file should be included in the
# APP_EXT_LIBS.mk file.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk


# LWIP lib dir
MBED_LWIP_LIB_DIR = $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface

MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip-sys/
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip-sys/arch/
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/api
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/core
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/core/ipv4
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/core/ipv6
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/netif
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/netif/ppp
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/netif/ppp/polarssl
# The next directories are only for include paths (i.e., header files)
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include/lwip
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include/lwip/priv
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include/lwip/prot
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include/netif
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include/netif/ppp
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip/src/include/netif/ppp/polarssl

#----------------------------- BOARD specific files ---------------------------#
ifeq ($(BOARD), EVAL_F469NI)
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip-eth/arch/TARGET_STM/TARGET_STM32F4/TARGET_EVAL_F469NI
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip-eth/arch/TARGET_STM

else ifeq ($(BOARD), K64F)
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip-eth/arch/TARGET_Freescale/TARGET_K64F
MBED_LWIP_LIB_DIR += $(MBED_LIBS_FEATURES_ROOT)FEATURE_LWIP/lwip-interface/lwip-eth/arch/TARGET_Freescale

# if board not found, through an error
else
$(error [ERROR]: MBED_LWIP Configurations for <$(BOARD)> has not been added.\
 Please add it in MBED_LWIP.mk file)

endif



# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
MBED_LWIP_LIB_C_SRCS = $(foreach dir,$(MBED_LWIP_LIB_DIR),$(wildcard $(dir)/*.c))
MBED_LWIP_LIB_CXX_SRCS = $(foreach dir,$(MBED_LWIP_LIB_DIR),$(wildcard $(dir)/*.cpp))

# Add the mbed LWIP directory to mbed libs dir
MBED_LIBS_DIRS += $(MBED_LWIP_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(MBED_LWIP_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(MBED_LWIP_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os
# or mbed-sdk).
