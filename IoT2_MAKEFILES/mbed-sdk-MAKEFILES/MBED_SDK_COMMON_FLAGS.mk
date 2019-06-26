
################################################################################
#
# This is an autoconfig file for commmon mbed-sdk flags of the application. 
#
################################################################################


#==========================     FLAGS    	  ==========================#

# COMMON flags
COMMON_FLAGS = -c
COMMON_FLAGS += -Wall
COMMON_FLAGS += -Wextra
COMMON_FLAGS += -Wno-unused-parameter
COMMON_FLAGS += -Wno-missing-field-initializers
COMMON_FLAGS += -fmessage-length=0
COMMON_FLAGS += -fno-exceptions
COMMON_FLAGS += -fno-builtin
COMMON_FLAGS += -ffunction-sections
COMMON_FLAGS += -fdata-sections
COMMON_FLAGS += -funsigned-char
COMMON_FLAGS += -MMD -MP -MF $(DEPDIR)$(@F:.o=.d)
COMMON_FLAGS += -fno-delete-null-pointer-checks
COMMON_FLAGS += -fomit-frame-pointer
COMMON_FLAGS += $(OPT_FLAG)
COMMON_FLAGS += -mcpu=cortex-m4
COMMON_FLAGS += -mthumb
COMMON_FLAGS += -mfpu=fpv4-sp-d16
COMMON_FLAGS += -mfloat-abi=softfp


# CC flags
C_FLAGS = -std=gnu99
C_FLAGS += $(COMMON_FLAGS)
C_FLAGS += -include
C_FLAGS += mbed_config.h

# IoT2 configuration
ifeq ($(IOT2), 1)
C_FLAGS += -include
C_FLAGS += IoT2Lib.h
endif 


# CXX flags
CXX_FLAGS = -std=gnu++98
CXX_FLAGS += -fno-rtti
CXX_FLAGS += -Wvla
CXX_FLAGS += $(COMMON_FLAGS)
CXX_FLAGS += -include
CXX_FLAGS += mbed_config.h

# IoT2 configuration
ifeq ($(IOT2), 1)
CXX_FLAGS += -include
CXX_FLAGS += IoT2Lib.h
endif

# ASSEMBLER FLAGS
ASM_FLAGS = $(COMMON_FLAGS)


# LD FLAGS
LD_FLAGS = -Wl,--gc-sections
LD_FLAGS += -Wl,--wrap,main
LD_FLAGS += -Wl,--wrap,exit
LD_FLAGS += -Wl,--wrap,atexit
LD_FLAGS += -Wl,-n
LD_FLAGS += -mcpu=cortex-m4
LD_FLAGS += -mthumb
LD_FLAGS += -mfpu=fpv4-sp-d16
LD_FLAGS += -mfloat-abi=softfp

# PREPROCESSOR FLAGS
PREPROC_FLAGS = -E
PREPROC_FLAGS += -P
PREPROC_FLAGS += $(LD_FLAGS)

# LIB flags
LIBS :=-Wl,--start-group -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -Wl,--end-group
