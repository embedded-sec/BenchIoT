
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


# The root of mbed specific libraries
MBED_LIBS_ROOT = $(EXT_LIB_ROOT)mbed-libs/

# The root of mbed features
MBED_LIBS_FEATURES_ROOT = $(MBED_LIBS_ROOT)FEATURES/

# The variable to hold the directories of all added libraries
MBED_LIBS_DIRS +=

# The *.c files of mbed-lib
MBED_LIBS_C_SRCS +=

# The *.cpp files of mbed-lib
MBED_LIBS_CXX_SRCS +=

