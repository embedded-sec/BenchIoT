
################################################################################
#
# This is used to automatically include the filesystem feature of mbed. If this
# feature is presesnt in the benchmark then this file should be included in the
# mbed-os makefile.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk


# Netsocket lib dir
MBED_NETSOCKET_LIB_DIR = $(MBED_LIBS_FEATURES_ROOT)netsocket


# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
MBED_NETSOCKET_LIB_C_SRCS = $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/*.c))
MBED_NETSOCKET_LIB_CXX_SRCS = $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/NetworkInterface.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/NetworkStack.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/Socket.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/SocketAddress.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/TCPServer.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/TCPSocket.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/UDPSocket.cpp))
MBED_NETSOCKET_LIB_CXX_SRCS += $(foreach dir,$(MBED_NETSOCKET_LIB_DIR),$(wildcard $(dir)/nsapi_dns.cpp))

# Add the mbed netsocket directory to mbed libs dir
MBED_LIBS_DIRS += $(MBED_NETSOCKET_LIB_DIR)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(MBED_NETSOCKET_LIB_C_SRCS)
MBED_LIBS_CXX_SRCS += $(MBED_NETSOCKET_LIB_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).